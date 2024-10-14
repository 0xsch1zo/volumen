#pragma once
#include <ftxui/screen/color.hpp>
#include <toml++/toml.hpp>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
namespace ft = ftxui;

class config {
    const std::string config_path;
    const std::string DEFAULT_CONFIG_MSG = "No config detected. Using defaults";
    const std::string LOAD_CONFIG_MSG = "Loading config from: ";
    
    toml::table config_toml;
    
    class colors {
        const toml::table config_toml;
        const std::string COLORS_GROUP = "colors";
        const ft::Color default_main = ft::Color::Green;
        const ft::Color default_accent_color1 = ft::Color::Red;
        const ft::Color default_accent_color2 = ft::Color::Violet;
        ft::Color main_color;
        ft::Color accent_color1;
        ft::Color accent_color2;
        struct rgb {
            int8_t red;
            int8_t green;
            int8_t blue;
        };

        ft::Color get_color(const std::string& key) const;
        rgb hextorgb(const std::string& hex) const;
        ft::Color parse_main_color() const;
        ft::Color parse_accent_color1() const;
        ft::Color parse_accent_color2() const;
    public:
        colors(const toml::table& config);
        ft::Color get_main_color() const;
        ft::Color get_accent_color1() const;
        ft::Color get_accent_color2() const;
    };

    class misc {
        const toml::table config_toml;
        const std::string MISC_GROUP = "misc";
        const std::string default_splash = R"(
 ██▒   █▓ ▒█████   ██▓     █    ██  ███▄ ▄███▓▓█████  ███▄    █ 
▓██░   █▒▒██▒  ██▒▓██▒     ██  ▓██▒▓██▒▀█▀ ██▒▓█   ▀  ██ ▀█   █ 
 ▓██  █▒░▒██░  ██▒▒██░    ▓██  ▒██░▓██    ▓██░▒███   ▓██  ▀█ ██▒
  ▒██ █░░▒██   ██░▒██░    ▓▓█  ░██░▒██    ▒██ ▒▓█  ▄ ▓██▒  ▐▌██▒
   ▒▀█░  ░ ████▓▒░░██████▒▒▒█████▓ ▒██▒   ░██▒░▒████▒▒██░   ▓██░
   ░ ▐░  ░ ▒░▒░▒░ ░ ▒░▓  ░░▒▓▒ ▒ ▒ ░ ▒░   ░  ░░░ ▒░ ░░ ▒░   ▒ ▒ 
   ░ ░░    ░ ▒ ▒░ ░ ░ ▒  ░░░▒░ ░ ░ ░  ░      ░ ░ ░  ░░ ░░   ░ ▒░
     ░░  ░ ░ ░ ▒    ░ ░    ░░░ ░ ░ ░      ░      ░      ░   ░ ░ 
      ░      ░ ░      ░  ░   ░            ░      ░  ░         ░ 
     ░                                                          
        )";
        std::string splash;

        std::string parse_splash() const;
    public:
        misc(const toml::table& config);
        std::string get_splash() const;
    };

    std::optional<misc> misc_o;
    std::optional<colors> colors_o;
protected:
    enum ERROR {
        INVALID_INT,
        INVALID_VAL,
        INVALID_HEX
    };    
    static std::string get_error_msg(ERROR e, const std::string& key);

public:
    config();
    misc Misc() const;
    colors Colors() const;
};