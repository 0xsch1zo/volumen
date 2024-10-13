#include "config.hpp"
#include <spdlog/spdlog.h>

namespace spd = spdlog;

config::config() {
    const std::string NAME = "volumen";
    const std::string CONFIG_NAME = "volumen_config.toml";
#if defined(_WIN32) || defined(_WIN64)
    char* appdata = std::getenv("APPDATA");  // C:\Users\YourUser\AppData\Roaming
    if(appdata)
        const_cast<std::string&>(config_path) = fs::path(appdata) / NAME / CONFIG_NAME;
#elif defined(__APPLE__)
    char* home = std::getenv("HOME");
    if(home)
        const_cast<std::string&>(config_path) = fs::path(home) / "Library" / "Preferences" / NAME / CONFIG_NAME;
#else  // Linux or other Unix-like
    char* home = std::getenv("HOME");
    if(home)
        const_cast<std::string&>(config_path) = fs::path(home) / ".config" / NAME / CONFIG_NAME;
#endif
    
    // If no config later calls on config_toml objectw will result in defaults being used
    if(!fs::exists(config_path)) {
        spd::debug(DEFAULT_CONFIG_MSG);
        return;
    }
        
    spd::debug(LOAD_CONFIG_MSG + config_path);
    config_toml = toml::parse_file(config_path);
}

std::string config::get_error_msg(ERROR e, const std::string& key) const {
    switch(e) {
        case INVALID_INT:
            return "Invalid intiger value for '" + key + "'. The value needs to be between 0 - 15";
        case INVALID_VAL:
            return "Invalid value for: '" + key + "'";
        case INVALID_HEX:
            return "Invalid hex value: '" + key + "'";
    }

    return "Unknown error";
}

ft::Color config::colors::get_color(const std::string& key) const {
    if(config_p->config_toml[COLORS_GROUP][key].is_integer()) {
        const int color16 = config_p->config_toml[COLORS_GROUP][key].value_or(-1);
        if(color16 < 0 || color16 >= 16)
            throw std::runtime_error(config_p->get_error_msg(INVALID_INT, key));
        return ft::Color::Palette16(color16);
    }
    else if(config_p->config_toml[COLORS_GROUP][key].is_string()) {
        rgb color_rgb = hextorgb(config_p->config_toml[COLORS_GROUP][key].value_or(""));
        return ft::Color(color_rgb.red, color_rgb.green, color_rgb.blue);
    }
    
    throw std::runtime_error(config_p->get_error_msg(INVALID_VAL, key));
}

config::colors::rgb config::colors::hextorgb(const std::string& hex) const {
    const int HEX_SIZE = 7;
    const int BEGIN = 1;
    const int END_RED = 2;
    const int END_GREEN = 4;
    const int END_BLUE = 6;
    const int BASE = 16;

    if(hex.size() != HEX_SIZE)
        throw std::runtime_error(config_p->get_error_msg(INVALID_HEX, hex));

    return {
        .red    = std::stoi(hex.substr(BEGIN, END_RED), 0, BASE),
        .green  = std::stoi(hex.substr(BEGIN, END_GREEN), 0, BASE),
        .blue   = std::stoi(hex.substr(BEGIN, END_BLUE), 0, BASE)
    };
}

ft::Color config::colors::get_main_color() const {
    const std::string main = "main";
    if(!config_p->config_toml[COLORS_GROUP][main].is_value())
        return default_main;
    else
        return get_color(main);
}

ft::Color config::colors::get_accent_color1() const {
    const std::string accent_color1 = "accent_color1";
    if(!config_p->config_toml[COLORS_GROUP][accent_color1].is_value())
        return default_accent_color1;
    else
        return get_color(accent_color1);
}

ft::Color config::colors::get_accent_color2() const {
    const std::string accent_color2 = "accent_color2";
    if(!config_p->config_toml[COLORS_GROUP][accent_color2].is_value())
        return default_accent_color2;
    else
        return get_color(accent_color2);
}

std::string config::misc::get_splash() const {
    const std::string splash_key = "splash";
    if(config_p->config_toml[MISC_GROUP][splash_key].is_string())
        return config_p->config_toml[MISC_GROUP][splash_key].value_or("");
    else if(config_p->config_toml[MISC_GROUP][splash_key].is_value())
        throw std::runtime_error(config_p->get_error_msg(INVALID_VAL, splash_key));
    else
        return default_splash;
}