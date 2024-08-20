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
    friend class authinterface;
    static const std::string LIBRUS_AUTHORIZE_URL;
    static const std::string LIBRUS_LOGIN_URL;
    static const std::string LIBRUS_APP_URL;
    static const std::string LIBRUS_OAUTH_URL;
    static const std::string LIBRUS_CLIENT_ID;
    static const std::string LIBRUS_API_ACCESS_TOKEN_URL;
    static const std::string redirectTo;
    static const std::string redirectCrc;
    static cl::Easy request;
    static struct oauth_data_t {
        std::string token_type;
        size_t expires_in;
        std::string access_token;
        std::string refresh_token;
    } oauth_data;
    static std::string get_authcode(std::string email, std::string password);
    static std::string find_token();
    static void write_func_cleanup();
    static void get_access_token(std::string authcode);
    static void get_accounts();
public:
    static bool auth_completed;
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
    static std::vector<synergia_account_t> synergia_accounts;
    static void authorize(std::string email, std::string password, bool& login_failed);
};

