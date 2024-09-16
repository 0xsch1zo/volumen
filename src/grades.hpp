#pragma once
#include "api.hpp"
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class grades {
    std::vector<char> grade_details_shown; // Treated as bool
    ft::Component modal_grade_details(const api::grade_t& grades_p);
public:
    void grades_display(ft::Component grades_component, api* api);
};