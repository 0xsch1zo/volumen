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

    static const int UNAUTHORIZED_ERR_CODE = 401;

public:
    auth(std::function<void()> refresh_token_failure_handler) : refresh_failure_handler(refresh_token_failure_handler) {}
    std::string get_api_access_token(const std::string& login);
    std::vector<synergia_account_t> get_synergia_accounts() const;
    void authorize(const std::string& email, const std::string& password);
    void refresh_api_tokens();
    void forget_refresh_token();

public:
    static const inline std::string login_service_field             = "login";
    static const inline std::string refresh_token_service_field     = "refresh_token";

private:
    struct oauth_data_t {
        std::string token_type;
        size_t expires_in;
        std::string access_token;
        std::string refresh_token;
    } oauth_data;
    
	const std::string LIBRUS_PORTAL_AUTHORIZE_URL           = "https://portal.librus.pl/konto-librus/redirect/dru";
	const std::string LIBRUS_PORTAL_LOGIN_URL               = "https://portal.librus.pl/konto-librus/login/action";
	const std::string LIBRUS_PORTAL_APP_URL                 = "app://librus";
	const std::string LIBRUS_PORTAL_OAUTH_URL               = "https://portal.librus.pl/oauth2/access_token";
	const std::string LIBRUS_API_ACCESS_TOKEN_URL           = "https://portal.librus.pl/api/v3/SynergiaAccounts";
	const std::string LIBRUS_API_ACCESS_TOKEN_FRESH_URL     = LIBRUS_API_ACCESS_TOKEN_URL + "/fresh";
	const std::string LIBRUS_PORTAL_CLIENT_ID               = "VaItV6oRutdo8fnjJwysnTjVlvaswf52ZqmXsJGP";
	const std::string redirectTo                            = "/konto-librus/redirect/dru";
	const std::string redirectCrc                           = "3b77fc51101d51dc0ae45dc34780a8a36c152daf307f454090ef6bb018a56fab";

    std::unordered_map<std::string, std::string> api_access_tokens;
    std::vector<synergia_account_t> synergia_accounts;
    std::function<void()> refresh_failure_handler;
    std::mutex access_token_mutex;

private:
    std::string find_token(cpr::Cookies& cookies);
    std::string get_authcode(const std::string& email, const std::string& password);
    void fetch_portal_access_token(const std::string& authcode);
    void fetch_synergia_accounts();
    bool refresh_portal_token();
};
