#pragma once
#include "api.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <memory>

namespace ft = ftxui;

class content {
protected:
    ft::ScreenInteractive* main_screen_p;
public:
    virtual void content_display(ft::Component content_component, api* api, size_t* redirect, std::mutex* redirect_mutex) = 0;
    virtual ft::Component content_view() = 0;
};