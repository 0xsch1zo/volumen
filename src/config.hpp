#pragma once
#include <ftxui/screen/color.hpp>
#include <toml++/toml.hpp>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
namespace ft = ftxui;

class config {
    const std::string config_path;
protected:
    enum ERROR {
            INVALID_INT,
            INVALID_VAL,
            INVALID_HEX
    };
    toml::table config_toml;

    std::string get_error_msg(ERROR e, const std::string& key) const;
public:
    config();
    const std::string DEFAULT_CONFIG_MSG = "No config detected. Using defaults";
    const std::string LOAD_CONFIG_MSG = "Loading config from: ";
    
    class colors {
        const config* config_p;
        const std::string COLORS_GROUP = "colors";
        const ft::Color default_main = ft::Color::Green;
        const ft::Color default_accent_color1 = ft::Color::Red;
        const ft::Color default_accent_color2 = ft::Color::Violet;

        

        struct rgb {
            int8_t red;
            int8_t green;
            int8_t blue;
        };

        ft::Color get_color(const std::string& key) const;
        rgb hextorgb(const std::string& hex) const;
    public:
        colors(const config* config) : config_p(config) {}
        ft::Color get_main_color() const;
        ft::Color get_accent_color1() const;
        ft::Color get_accent_color2() const;
    };

    class misc {
        const config* config_p;
        const std::string MISC_GROUP = "misc";
        const std::string default_splash = "default splash";
    public:
        misc(const config* config) : config_p(config) {}
        std::string get_splash() const;
    };
};