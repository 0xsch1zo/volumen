#pragma once
#include <string>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

namespace spd = spdlog;
namespace cl = cURLpp;
using json = nlohmann::json;

class authorization {
    static const std::string LIBRUS_PORTAL_AUTHORIZE_URL;
    static const std::string LIBRUS_PORTAL_LOGIN_URL;
    static const std::string LIBRUS_PORTAL_APP_URL;
    static const std::string LIBRUS_PORTAL_OAUTH_URL;
    static const std::string LIBRUS_PORTAL_CLIENT_ID;
    static const std::string LIBRUS_API_ACCESS_TOKEN_URL;
    static const std::string redirectTo;
    static const std::string redirectCrc;
    struct oauth_data_t {
        std::string token_type;
        size_t expires_in;
        std::string access_token;
        std::string refresh_token;
    };
public:
    class synergia_account_t {
    private:
        friend class authorization;
        friend class api;
        std::string access_token;
    public:
        std::string group;
        std::string student_name;
        std::string login;
    };

private:
    static std::string get_authcode(std::string email, std::string password, cl::Easy& request);
    static std::string find_token(cl::Easy& request);
    static oauth_data_t get_portal_access_token(std::string authcode, cl::Easy& request);
public:
    static bool authorize(std::string email, std::string password);
    static std::vector<synergia_account_t>& get_synergia_accounts(oauth_data_t* oauth_data = nullptr, cl::Easy* request = nullptr);
};

