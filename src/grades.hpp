#pragma once
#include "api.hpp"
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class grades {

public:
    void grades_display(ft::Component grades_component, api* api);
};