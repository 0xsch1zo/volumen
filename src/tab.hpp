#pragma once
#include "auth.hpp"
#include "error_handler.hpp"
#include "config.hpp"
#include <ftxui/component/component.hpp>

class tab {
    const config* config_p;
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
    
    tab(const config* config) : config_p(config) {}
    void display_interface(const auth& auth_o, const std::string& picked_login);
};