#include "authorization.hpp"
#include "utils.hpp"
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <regex>

const std::string authorization::LIBRUS_PORTAL_AUTHORIZE_URL           = "https://portal.librus.pl/konto-librus/redirect/dru";
const std::string authorization::LIBRUS_PORTAL_LOGIN_URL               = "https://portal.librus.pl/konto-librus/login/action";
const std::string authorization::LIBRUS_PORTAL_APP_URL                 = "app://librus";
const std::string authorization::LIBRUS_PORTAL_OAUTH_URL               = "https://portal.librus.pl/oauth2/access_token";
const std::string authorization::LIBRUS_API_ACCESS_TOKEN_URL    = "https://portal.librus.pl/api/v3/SynergiaAccounts";
const std::string authorization::LIBRUS_PORTAL_CLIENT_ID               = "VaItV6oRutdo8fnjJwysnTjVlvaswf52ZqmXsJGP";
const std::string authorization::redirectTo                     = "/konto-librus/redirect/dru";
const std::string authorization::redirectCrc                    = "3b77fc51101d51dc0ae45dc34780a8a36c152daf307f454090ef6bb018a56fab";

// TODO: make custom exception type for login
// TODO: make shared_request_opt_setup
bool authorization::authorize(std::string email, std::string password) {
    std::string authcode;
    cl::Easy request;
    request.setOpt<cl::options::CookieFile>("");
    request.setOpt<cl::options::FollowLocation>(true);
    request.setOpt(cl::options::Verbose(false));
    request.setOpt<cl::options::AutoReferer>(false);

    try {
        // if login failed return
        authcode = get_authcode(email, password, request);
        oauth_data_t oauth_data = std::move(get_portal_access_token(authcode, request));
        get_synergia_accounts(&oauth_data, &request);
    }
    catch(cl::RuntimeError &e) {
		spd::error("Request failed with exception: {}", e.what());
        goto common_err_stub;
	}
	catch(cl::LogicError &e) {
		spd::error("Request failed with exception: {}", e.what());
        goto common_err_stub;
	}
    catch (std::logic_error &e) {
        spd::error(e.what());
        goto common_err_stub;
    }
    catch (std::runtime_error &e) {
        spd::error(e.what());
        goto common_err_stub;
    }
    catch (json::type_error &e) {
        spd::error(e.what());
        goto common_err_stub;
    }
    catch (json::parse_error &e) {
        spd::error(e.what());
        goto common_err_stub;
    }
    catch (json::other_error &e) {
        spd::error(e.what());
        goto common_err_stub;
    }
    return true;

common_err_stub:
    return false;
}

// After a request is sent with the bearer token for portal.librus.pl the endpoint will provide all Synergia accounts(and access tokens to said accounts) asociated with the konto librus acc
// ARGUMENTS CAN BE NULLPTRS ONLY AFTER FIRST CALL
std::vector<authorization::synergia_account_t>& authorization::get_synergia_accounts(oauth_data_t* oauth_data, cl::Easy* request) {
    static std::vector<synergia_account_t> synergia_accounts; 
    // If we already have populated synergia_accounts just return reference. Works because static
    if(!synergia_accounts.empty()) {
        return synergia_accounts;
    }

    assert(oauth_data == nullptr || request == nullptr); // Arguments can't be nullptrs on first call
    std::ostringstream os;
    request->setOpt<cl::options::WriteStream>(&os);
    request->setOpt<cl::options::Url>(LIBRUS_API_ACCESS_TOKEN_URL);
    std::list<std::string> auth = {"Authorization: Bearer " + oauth_data->access_token};
    request->setOpt<cl::options::HttpHeader>(auth);
    request->perform();
    json data = json::parse(os.str());
    // TODO: If fails log where it happened
    for(auto& account : data["accounts"].items()) {
        synergia_account_t acc;
        acc.group = account.value()["group"];
        acc.access_token = account.value()["accessToken"];
        acc.student_name = account.value()["studentName"];
        acc.login = account.value()["login"];
        synergia_accounts.push_back(acc);
    }
    
    return synergia_accounts;
}

authorization::oauth_data_t authorization::get_portal_access_token(std::string authcode, cl::Easy& request) {
    std::ostringstream os;
    request.setOpt<cl::options::Url>(LIBRUS_PORTAL_OAUTH_URL);
    cl::Forms access_token_data; 
    access_token_data.push_back(new cl::FormParts::Content("grant_type", "authorization_code"));
    access_token_data.push_back(new cl::FormParts::Content("client_id", LIBRUS_PORTAL_CLIENT_ID));
    access_token_data.push_back(new cl::FormParts::Content("redirect_uri", LIBRUS_PORTAL_APP_URL));
    access_token_data.push_back(new cl::FormParts::Content("code", authcode));
    request.setOpt(cl::options::HttpPost(access_token_data));
    request.setOpt<cl::options::WriteStream>(&os);
    request.perform();

    oauth_data_t oauth_data;
    json data = json::parse(os.str());
    try {
        oauth_data = { 
            .token_type     = data["token_type"],
            .expires_in     = data["expires_in"],
            .access_token   = data["access_token"],
            .refresh_token  = data["refresh_token"]
        };
    }
    catch (json::type_error &e) {
        // TODO: handle properly json fail
        // Add some diagnostics on top of the thrown exception
        // If this fails authorization::authorize() will catch it
        spd::critical(
            "error: {}\nerror_description: {}\nhint: {}\nmessage: {}", 
            std::string(data["error"]), 
            std::string(data["error_description"]), 
            std::string(data["hint"]),
            std::string(data["message"])
        );
        throw;
    }

    // Cleanup
    request.setOpt<cl::options::HttpGet>(true);
    write_func_cleanup(request);
    return oauth_data;
}

std::string authorization::find_token(cl::Easy& request) {
    const int TOKEN_SIZE = 40; 
    std::ostringstream os;
    request.setOpt<cl::options::Url>(LIBRUS_PORTAL_AUTHORIZE_URL);
    request.setOpt<cl::options::WriteStream>(&os);
    request.perform();

    std::string resp = os.str();

    std::regex token_regex("^.*input.*_token.*value=\"", std::regex::grep);
    std::smatch token_match;

    if(!std::regex_search(resp, token_match, token_regex))
        throw std::logic_error("Couldn't find _token in response:\n"  + resp);

    std::string token_suffix = token_match.suffix();

    // Cleanup after execution else segfault wiil occur becuase os goes out of scope
    write_func_cleanup(request);

    return token_suffix.substr(0,TOKEN_SIZE);
}

// TODO: Limit allowed protocols because of redirects
std::string authorization::get_authcode(std::string email, std::string password, cl::Easy& request) {
    std::string _token = find_token(request);

    // Fomrs takes ownership of the pointers
    cl::Forms authcode_data;
    authcode_data.push_back(new cl::FormParts::Content("redirectTo",     redirectTo));
    authcode_data.push_back(new cl::FormParts::Content("redirectCrc",    redirectCrc));
    authcode_data.push_back(new cl::FormParts::Content("email",          email));
    authcode_data.push_back(new cl::FormParts::Content("password",       password));
    authcode_data.push_back(new cl::FormParts::Content("_token",         _token));

    // Get the headers
    std::string authcode;

    request.setOpt<cl::options::Url>(LIBRUS_PORTAL_LOGIN_URL);
    request.setOpt(cl::options::HttpPost(authcode_data));
    request.setOpt(cl::options::HeaderFunction([&authcode] (char* buffer, size_t size, size_t items) {
        size_t incoming_size = size * items;
        std::string header(buffer, incoming_size); // buffer is not null terminated

        std::regex authcode_regex("location:.*" + LIBRUS_PORTAL_APP_URL + "?code=", std::regex::grep);
        std::smatch authcode_match;
        if(!std::regex_search(header, authcode_match, authcode_regex)) return incoming_size;

        const std::string authcode_suffix = authcode_match.suffix();
        authcode = authcode_suffix.substr(0, authcode_suffix.find("&"));
        return incoming_size;
    }));

    try {
        request.perform();
    } catch (cl::RuntimeError &e) {
        const std::string desired_execption = "The redirect target URL could not be parsed: Unsupported URL scheme";
        const std::string what_was_thrown = e.what();

        if(desired_execption != what_was_thrown)
            throw;
    }

    if(authcode == "") throw std::runtime_error("LOGIN FAILURE: Couldn't find authorization code(most likely wrong email or passoword)");
    return authcode;
}