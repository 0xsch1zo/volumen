#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include "../timetable.hpp"

namespace ft = ftxui;

class dashboard {
    class timetable_dashboard {
        static ft::Element get_timeline_widget(const std::vector<api::lesson_t>& day);
        bool active{};
    public:
        ft::Component get_timetable_widget(api* api);
    };

    class grades_dashboard {
        bool active{};
    public:
        ft::Component get_grades_widget(api* api);
    };

    class weekend_bar {
    public:
        static ft::Component get_weekend_bar(api* api);
    };

public:
    static void dashboard_display(ft::Component dashboard_component, api* api);
};
