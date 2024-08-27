#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ft = ftxui;

class login {
    login();
    static const std::string splash;
    static ft::ScreenInteractive screen;
    static ft::ButtonOption button_style();

public:
    static void login_screen();
    static void choose_account_screen();
};