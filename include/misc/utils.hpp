#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <api/api.hpp>

namespace ft = ftxui;

typedef std::function<bool(ft::Event)> event_handler;

#define VAR_NAME(x) #x

class utils {
    utils();
public:
    static ft::Elements split(std::string text);
    static int get_day_of_week(std::string&& date_unformated);
    static std::function<bool(ft::Event)> exit_active_screen_on_keybind();
};