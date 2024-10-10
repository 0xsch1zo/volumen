#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

using json = nlohmann::json;

class auth {
public:
    struct synergia_account_t {
        std::string group;
        std::string student_name;
        std::string login;
    };

public:
    std::string get_api_access_token(const std::string& login) const;
    std::vector<synergia_account_t> get_synergia_accounts() const;
    void authorize(const std::string& email, const std::string& password);

private:
    struct oauth_data_t {
        std::string token_type;
        size_t expires_in;
        std::string access_token;
        std::string refresh_token;
    };
    
	const std::string LIBRUS_PORTAL_AUTHORIZE_URL           = "https://portal.librus.pl/konto-librus/redirect/dru";
	const std::string LIBRUS_PORTAL_LOGIN_URL               = "https://portal.librus.pl/konto-librus/login/action";
	const std::string LIBRUS_PORTAL_APP_URL                 = "app://librus";
	const std::string LIBRUS_PORTAL_OAUTH_URL               = "https://portal.librus.pl/oauth2/access_token";
	const std::string LIBRUS_API_ACCESS_TOKEN_URL           = "https://portal.librus.pl/api/v3/SynergiaAccounts";
	const std::string LIBRUS_PORTAL_CLIENT_ID               = "VaItV6oRutdo8fnjJwysnTjVlvaswf52ZqmXsJGP";
	const std::string redirectTo                            = "/konto-librus/redirect/dru";
	const std::string redirectCrc                           = "3b77fc51101d51dc0ae45dc34780a8a36c152daf307f454090ef6bb018a56fab";

    std::unordered_map<std::string, std::string> api_access_tokens;
    std::vector<synergia_account_t> synergia_accounts;

private:
    std::string find_token(cpr::Cookies& cookies);
    std::string get_authcode(const std::string& email, const std::string& password);
    oauth_data_t fetch_portal_access_token(const std::string& authcode);
    void fetch_synergia_accounts(const oauth_data_t& oauth_data);
};


