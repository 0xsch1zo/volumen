#include <auth/auth.hpp>
#include <misc/ssave.hpp>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <regex>

void auth::authorize(const std::string& email, const std::string& password) {
    const std::string authcode = get_authcode(email, password);
    fetch_portal_access_token(authcode);
    fetch_synergia_accounts();
}

std::string auth::get_api_access_token(const std::string& login) const {
    assert(api_access_tokens.count(login));
    return api_access_tokens.at(login);
}

std::vector<auth::synergia_account_t> auth::get_synergia_accounts() const {
    assert(!synergia_accounts.empty());
    return synergia_accounts;
}

void auth::forget_refresh_token() {
    oauth_data.refresh_token.clear();
}

// After a request is sent with the bearer token for portal.librus.pl the endpoint will provide all Synergia accounts(and access tokens to said accounts) asociated with the konto librus acc
void auth::fetch_synergia_accounts() {
    cpr::Response r = cpr::Get(
        cpr::Url{LIBRUS_API_ACCESS_TOKEN_URL},
        cpr::Bearer{oauth_data.access_token}
    );

    json data = json::parse(r.text);

    // In case already populated and called on refresh
    synergia_accounts.clear();
    api_access_tokens.clear();

    synergia_accounts.reserve(data["accounts"].size());
    api_access_tokens.reserve(data["accounts"].size());

    // TODO: If fails log where it happened
    for(const auto& account : data["accounts"].items()) {
        synergia_accounts.emplace_back(
            /*.group = */           account.value()["group"],
            /*.student_name = */    account.value()["studentName"],
            /*.login = */           account.value()["login"]
        );

        api_access_tokens.emplace(
            account.value()["login"],
            account.value()["accessToken"]
        );
    }
}

bool auth::refresh_portal_token() {
    std::string refresh_token;
    if(oauth_data.refresh_token.empty() && ssave::exists(refresh_token_service_field)) {
        refresh_token = ssave::get(refresh_token_service_field);
    } else {
        refresh_token = oauth_data.refresh_token;
    }

    cpr::Multipart refresh_token_data {
        { "grant_type",      "refresh_token" },
        { "client_id",       LIBRUS_PORTAL_CLIENT_ID },
        { "redirect_uri",    LIBRUS_PORTAL_APP_URL },
        { "refresh_token",   refresh_token }
    };
    cpr::Response r = cpr::Post(
        cpr::Url{ LIBRUS_PORTAL_OAUTH_URL },
        refresh_token_data 
    );

    json data = json::parse(r.text);

    // If the refresh token has expired begin oauth flow from start
    if(r.status_code >= 400) {
        refresh_failure_handler();
        return false;
    }

    oauth_data = {
        .token_type     = data["token_type"],
        .expires_in     = data["expires_in"],
        .access_token   = data["access_token"],
        .refresh_token  = data["refresh_token"]
    };

    ssave::save(oauth_data.refresh_token, refresh_token_service_field);
    return true;
}

void auth::refresh_api_tokens() {
    // Refresh the portal access token
    if(refresh_portal_token()) {
        // Renew api tokens if refreshing the access token was successful
        // If oauth flow had to be started all over we don't care about refreshing it now it will get called anyway
        fetch_synergia_accounts();
    }
}

void auth::fetch_portal_access_token(const std::string& authcode) {
    cpr::Multipart access_token_data {
        { "grant_type",      "authorization_code" },
        { "client_id",       LIBRUS_PORTAL_CLIENT_ID },
        { "redirect_uri",    LIBRUS_PORTAL_APP_URL },
        { "code",            authcode }
    };
    cpr::Response r = cpr::Post(
        cpr::Url{ LIBRUS_PORTAL_OAUTH_URL },
        access_token_data
    );

    json data = json::parse(r.text);

    oauth_data = { 
        .token_type     = data["token_type"],
        .expires_in     = data["expires_in"],
        .access_token   = data["access_token"],
        .refresh_token  = data["refresh_token"]
    };

    ssave::save(oauth_data.refresh_token, refresh_token_service_field);
}

std::string auth::find_token(cpr::Cookies& cookies) {
    const int TOKEN_SIZE = 40; 
    cpr::Response r = cpr::Get(cpr::Url(LIBRUS_PORTAL_AUTHORIZE_URL));

    cookies = r.cookies;

    std::regex token_regex("^.*input.*_token.*value=\"", std::regex::grep);
    std::smatch token_match;

    if(!std::regex_search(r.text, token_match, token_regex))
        throw std::logic_error("Couldn't find _token in response:\n"  + r.text);

    std::string token_suffix = token_match.suffix();

    return token_suffix.substr(0,TOKEN_SIZE);
}

std::string auth::get_authcode(const std::string& email, const std::string& password) {
    cpr::Cookies cookies;
    std::string _token = find_token(cookies);

	cpr::Response r = cpr::Post(
		cpr::Url{LIBRUS_PORTAL_LOGIN_URL},
		cpr::Payload{
			{ "redirectTo", 	redirectTo	},
			{ "redirectCrc", 	redirectCrc	},
			{ "email", 			email		},
			{ "password", 		password	},
			{ "_token", 		_token		}
		},
        cpr::Redirect(cpr::PostRedirectFlags::POST_301), // To respect RFC(I don't really care but, why not)
        cookies
	);

    // Search for authcode in "location" header
    std::string authcode;
    std::regex authcode_regex(LIBRUS_PORTAL_APP_URL + "?code=", std::regex::grep);
    std::smatch authcode_match;

    if(!std::regex_search(r.header["location"], authcode_match, authcode_regex))
        throw std::runtime_error("LOGIN FAILURE: Couldn't find auth code(most likely wrong email or passoword)");
	
    const std::string authcode_suffix = authcode_match.suffix();
    authcode = authcode_suffix.substr(0, authcode_suffix.find('&'));

    return authcode;
}
