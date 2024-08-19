#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>

namespace ft = ftxui;

class login {
    login();
    static ft::Elements split(std::string text);
    static ft::ButtonOption button_style();

public:
    static void ui();
};