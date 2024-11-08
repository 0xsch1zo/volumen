#pragma once
#include <auth/auth.hpp>
#include <misc/error_handler.hpp>
#include <misc/config.hpp>
#include <ftxui/component/component.hpp>

class main_ui {
    const config* config_p;
    enum tabs_t {
        DASHBOARD,
        MESSAGES,
        ANNOUCEMENTS,
        TIMETABLE,
        GRADES
    };

    static void ui_error_wrapper(
        error* e, 
        std::function<void()> caller_lambda, 
        ft::Component tab_container, 
        main_ui::tabs_t tabs
    );

public:
    enum ui_screens {
        MESSAGE_VIEW,
        ANNOUCEMENT_VIEW,
        EXIT
    };
    
    main_ui(const config* config) : config_p(config) {}
    bool display_interface(auth& auth_o, const std::string& picked_login);
};