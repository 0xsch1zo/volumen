#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

void display();
class homescreen {
    static ft::Component get_timetable_widget(std::vector<std::string>& lessons, std::function<void()> on_change);
    static ft::Component get_timeline_widget(std::vector<std::string>& timeline);
public:
    static ft::Component homescreen_tab(std::vector<std::string>& timeline, std::vector<std::string>& lessons, int& selected, std::function<void()> on_enter);
};