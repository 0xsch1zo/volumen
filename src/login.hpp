#pragma once
#include "auth.hpp"
#include "config.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ft = ftxui;

class login {
    ft::ScreenInteractive screen = ft::ScreenInteractive::Fullscreen();
    const config* config_p;
public:
    login(const config* config) : config_p(config) {}
    void login_screen(auth& auth_o);
    void choose_account_screen(const auth& auth_o);
};