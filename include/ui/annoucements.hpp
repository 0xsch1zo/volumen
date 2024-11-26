#pragma once
#include <api/api.hpp>
#include <bitset>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <memory>

namespace ft = ftxui;

class annoucements { 
    int selected_{};
    api::annoucements_t annoucements_;
public:
    void content_display(ft::Component content_component, api* api, size_t* redirect, std::mutex* redirect_mutex);
    ft::Component content_view();
};