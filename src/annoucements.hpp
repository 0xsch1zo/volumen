#pragma once
#include "api.hpp"
#include "content.hpp"
#include <bitset>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <memory>

namespace ft = ftxui;

class annoucements : public content { 
    size_t selected{};
    api::annoucements_t annoucements_o;
public:
    annoucements(ftxui::ScreenInteractive* main_screen);
    void content_display(ft::Component content_component, api* api, size_t* redirect, std::mutex* redirect_mutex) override;
    ft::Component content_view() override;
};