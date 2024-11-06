#pragma once
#include <string>

class ssave {
public:
    static void save(const std::string& secret, const std::string& service);
    static std::string get(const std::string& service);
    static bool exists(const std::string& service);
private:
    static const inline std::string package = "volumen";
    static const inline std::string user = getenv("USER");
};