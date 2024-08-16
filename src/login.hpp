#pragma once
#include <ftxui/dom/elements.hpp>

namespace ft = ftxui;

class login {
    static ft::Elements split(std::string text);
public:
    login();
    static void ui();
};