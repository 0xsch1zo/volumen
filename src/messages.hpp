#pragma once
#include "api.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>

namespace ft = ftxui;

class messages {
    ft::ScreenInteractive* main_screen_p;
    int selected{};
    api::messages_t messages_o;
public:
    messages(ft::ScreenInteractive* main_screen) : main_screen_p(main_screen) {}
    void content_display(ft::Component content_component, api* api, size_t* redirect, std::mutex* redirect_mutex);
    ft::Component content_view(); 
};