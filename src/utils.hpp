#pragma once
#include <curlpp/Easy.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include "api.hpp"

namespace ft = ftxui;

class utils {
public:
    static ft::Elements split(std::string text);
    static int get_day_of_week(std::string&& date_unformated);
    static void write_func_cleanup(curlpp::Easy& request);
    static ft::ButtonOption button_rounded();
    static ft::Component custom_component_window(ft::Element title, ft::Component contents);
    static ft::InputOption plain_input();
    static ft::Component custom_dropdown(ft::ConstStringListRef entries, int* selected);
};