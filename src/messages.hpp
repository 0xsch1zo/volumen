#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <string>
#include <vector>

namespace ft =  ftxui;

class messages {
public:
    struct message_t {
        std::string subject;
        std::string content;
        std::string author;
    };
    static ft::Component message_box(std::vector<message_t>& messages, int& message_chosen, std::function<void()> on_enter);
};