#pragma once
#include "api.hpp"
#include <bitset>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <memory>

namespace ft = ftxui;

class annoucements { 
    ft::ScreenInteractive* main_screen_p;
    size_t selected{};
    api::annoucements_t annoucements_o;
public:
    annoucements(ft::ScreenInteractive* main_screen) : main_screen_p(main_screen) {}
    void content_display(ft::Component content_component, api* api, size_t* redirect, std::mutex* redirect_mutex);
    ft::Component content_view();
};