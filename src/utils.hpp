#pragma once
#include <curlpp/Easy.hpp>
#include <ftxui/dom/elements.hpp>

namespace ft = ftxui;

class utils {
public:
    static void write_func_cleanup(curlpp::Easy& request);
    static ft::Elements split(std::string text);
};