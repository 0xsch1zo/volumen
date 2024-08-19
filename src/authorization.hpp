#pragma once
#include <string>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <spdlog/spdlog.h>

namespace spd = spdlog;
namespace cl = cURLpp;

class authorization {
    static const std::string LIBRUS_AUTHORIZE_URL;
    static const std::string LIBRUS_LOGIN_URL;
    static const std::string LIBRUS_APP_URL;
    static const std::string LIBRUS_OAUTH_URL;
    static const std::string LIBRUS_CLIENT_ID;
    static const std::string redirectTo;
    static const std::string redirectCrc;
    static std::string get_authcode(std::string email, std::string password, cl::Easy *request);
    static std::string find_token(cl::Easy* request);
    static void write_func_cleanup(cl::Easy* request);
    static void get_access_token(std::string authcode, cl::Easy* request);
public:
    static void authorize(std::string email, std::string password, bool& login_failed);
};