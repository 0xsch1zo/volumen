#include "authorization.hpp"
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <regex>

const std::string authorization::LIBRUS_AUTHORIZE_URL    = "https://portal.librus.pl/konto-librus/redirect/dru";
const std::string authorization::LIBRUS_LOGIN_URL        = "https://portal.librus.pl/konto-librus/login/action";
const std::string authorization::LIBRUS_APP_URL          = "app://librus";
const std::string authorization::LIBRUS_OAUTH_URL        = "https://portal.librus.pl/oauth2/access_token";
const std::string authorization::LIBRUS_CLIENT_ID        = "VaItV6oRutdo8fnjJwysnTjVlvaswf52ZqmXsJGP";
const std::string authorization::redirectTo              = "/konto-librus/redirect/dru";
const std::string authorization::redirectCrc             = "3b77fc51101d51dc0ae45dc34780a8a36c152daf307f454090ef6bb018a56fab";

// Cleanup after execution else segfault wiil occur becuase os goes out of scope
void authorization::write_func_cleanup(cl::Easy* request) {
    request->setOpt(cl::options::WriteFunction([](char* data, size_t size, size_t nmemb) {
        return size * nmemb;
    }));
}

void authorization::get_access_token(std::string authcode, cl::Easy* request) {
    std::ostringstream os;
    request->setOpt<cl::options::Url>(LIBRUS_OAUTH_URL);
    cl::Forms access_token_data; 
    access_token_data.push_back(new cl::FormParts::Content("grant_type", "authorization_code"));
    access_token_data.push_back(new cl::FormParts::Content("client_id", LIBRUS_CLIENT_ID));
    access_token_data.push_back(new cl::FormParts::Content("redirect_uri", LIBRUS_APP_URL));
    access_token_data.push_back(new cl::FormParts::Content("code", authcode));
    request->setOpt(cl::options::HttpPost(access_token_data));
    request->setOpt<cl::options::WriteStream>(&os);
    request->perform();

    write_func_cleanup(request);
}

// TODO: make custom exception type for login
void authorization::authorize(std::string email, std::string password, bool& login_failed) {
    cl::Easy request;
    std::string authcode;
    request.setOpt<cl::options::CookieFile>("");
    request.setOpt<cl::options::FollowLocation>(true);
    request.setOpt(cl::options::Verbose(true));
    request.setOpt<cl::options::AutoReferer>(false);

    try {
        // if login failed return
        authcode = get_authcode(email, password, &request);
        get_access_token(authcode, &request);
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

    cl::Cleanup();
    return;

common_err_stub:
    login_failed = true;
    cl::Cleanup();
    return;
}

std::string authorization::find_token(cl::Easy* request) {
    const int TOKEN_SIZE = 40; 
    std::ostringstream os;
    request->setOpt<cl::options::Url>(LIBRUS_AUTHORIZE_URL);
    request->setOpt<cl::options::WriteStream>(&os);
    request->perform();

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
std::string authorization::get_authcode(std::string email, std::string password, cl::Easy *request) {
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

    request->setOpt<cl::options::Url>(LIBRUS_LOGIN_URL);
    request->setOpt(cl::options::HttpPost(authcode_data));
    request->setOpt(cl::options::HeaderFunction([&authcode] (char* buffer, size_t size, size_t items) {
        size_t incoming_size = size * items;
        std::string header(buffer, incoming_size); // buffer is not null terminated

        std::regex authcode_regex("location:.*" + LIBRUS_APP_URL + "?code=", std::regex::grep);
        std::smatch authcode_match;
        if(!std::regex_search(header, authcode_match, authcode_regex)) return incoming_size;

        const std::string authcode_suffix = authcode_match.suffix();
        authcode = authcode_suffix.substr(0, authcode_suffix.find("&"));
        return incoming_size;
    }));

    try {
        request->perform();
    } catch (cl::RuntimeError &e) {
        const std::string desired_execption = "The redirect target URL could not be parsed: Unsupported URL scheme";
        const std::string what_was_thrown = e.what();

        if(desired_execption != what_was_thrown)
            throw;
    }

    if(authcode == "") throw std::runtime_error("Couldn't find authorization code");
    return authcode;
}