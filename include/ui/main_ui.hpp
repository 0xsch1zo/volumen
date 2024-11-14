#pragma once
#include <auth/auth.hpp>
#include <misc/error_handler.hpp>
#include <misc/config.hpp>
#include <misc/utils.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

class main_ui {
    const config* config_p;
    ft::ScreenInteractive main_screen = ft::ScreenInteractive::Fullscreen();
    auth& auth_;

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

    event_handler logout_handler(bool& logout);
    event_handler navigation_handler(int& tab_selected);
public:
    enum ui_screens {
        MESSAGE_VIEW,
        ANNOUCEMENT_VIEW,
        EXIT
    };
    
    main_ui(const config* config, auth& auth) : 
        config_p(config),
        auth_(auth) 
    {}
    bool display_interface(const std::string& picked_login);
};