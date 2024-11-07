#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include "api.hpp"

namespace ft = ftxui;

class utils {
    utils();
public:
    static ft::Elements split(std::string text);
    static int get_day_of_week(std::string&& date_unformated);
    static std::function<bool(ft::Event)> exit_on_keybind(const std::function<void()>& screen_exit);
};