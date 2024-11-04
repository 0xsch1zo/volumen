#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include "api.hpp"

namespace ft = ftxui;

class utils {
    struct length_and_res {
        int res;
        int length;
    };

    struct ansi_color {
        ft::Color foreground = ft::Color::Default;
        ft::Color background = ft::Color::Default;
    };

    static length_and_res find_longest_numeric_secuence(const std::string& num);
    static ansi_color get_ansi_color(std::string&& escape_sequence);
public:
    static ft::Elements ansi_color_text_area(const std::string& text);
    static ft::Elements split(std::string text);
    static int get_day_of_week(std::string&& date_unformated);
};