#pragma once
#define NOMINMAX
#include <api/api.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>

namespace ft = ftxui;

class messages {
    int message_selected_{};
    int messages_type_selected_{};
    api::messages_t messages_;
public:
    void content_display(ft::Component content_component, api* api, size_t* redirect, std::mutex* redirect_mutex);
    ft::Component content_view(); 
};