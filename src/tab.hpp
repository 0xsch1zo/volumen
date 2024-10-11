#pragma once
#include "auth.hpp"
#include "error_handler.hpp"
#include <ftxui/component/component.hpp>

class tab {
    enum tabs_t {
        DASHBOARD,
        MESSAGES,
        ANNOUCEMENTS,
        TIMETABLE,
        GRADES
    };

    static void tab_error_wrapper(
        error* e, 
        std::function<void()> caller_lambda, 
        ft::Component tab_container, 
        tab::tabs_t tabs
    );

public:
    enum ui_screens {
        MESSAGE_VIEW,
        ANNOUCEMENT_VIEW,
        EXIT
    };
    
    static void display_interface(const auth& auth_o, const std::string& picked_login);
};