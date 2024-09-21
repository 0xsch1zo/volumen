#pragma once
#include <curlpp/Easy.hpp>
#include <ftxui/dom/elements.hpp>
#include "api.hpp"

namespace ft = ftxui;

class utils {
public:
    static ft::Elements split(std::string text);
    static int get_day_of_week(std::string&& date_unformated);
    static void write_func_cleanup(curlpp::Easy& request);
};