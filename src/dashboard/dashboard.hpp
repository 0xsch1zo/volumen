#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class dashboard {
    class timetable {
    public:
        static ft::Component get_timeline_widget(std::vector<std::string>& timeline);
        static ft::Component get_timetable_widget(std::vector<std::string>& lessons, std::function<void()> on_change);
    };
public:
    static ft::Component dashboard_display(std::vector<std::string>& timeline, std::vector<std::string>& lessons, int& selected, std::function<void()> on_enter);
};