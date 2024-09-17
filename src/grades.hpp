#pragma once
#include "api.hpp"
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class grades {
    ft::Component grade_box(const api::grade_t& grades_p);
    ft::Component empty_subject(const std::string& subject);
public:
    void grades_display(ft::Component grades_component, api* api);
};