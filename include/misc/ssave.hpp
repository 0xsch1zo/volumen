#pragma once
#include <string>

class ssave {
public:
    static std::string get(const std::string& service);
    static void save(const std::string& secret, const std::string& service);
    static bool exists(const std::string& service);
    static void del(const std::string& service);

private:
    ssave();
    static const inline std::string package_    = "volumen";
#ifdef _WIN32
    static const inline std::string user_       = std::getenv("UserName");
#else
    static const inline std::string user_       = std::getenv("USER");
#endif
};