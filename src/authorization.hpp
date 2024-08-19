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
    static const std::string LIBRUS_AUTHORIZE_URL;
    static const std::string LIBRUS_LOGIN_URL;
    static const std::string LIBRUS_APP_URL;
    static const std::string LIBRUS_OAUTH_URL;
    static const std::string LIBRUS_CLIENT_ID;
    static const std::string redirectTo;
    static const std::string redirectCrc;
    static struct oauth_data_t {
        std::string token_type;
        size_t expires_in;
        std::string access_token;
        std::string refresh_token;
    } oauth_data;
    static std::string get_authcode(std::string email, std::string password, cl::Easy *request);
    static std::string find_token(cl::Easy* request);
    static void write_func_cleanup(cl::Easy* request);
    static oauth_data_t* get_access_token(std::string authcode, cl::Easy* request, oauth_data_t* oauth_data);

    public:
    static void authorize(std::string email, std::string password, bool& login_failed);
};