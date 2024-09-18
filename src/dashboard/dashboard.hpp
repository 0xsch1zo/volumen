#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include "../timetable.hpp"

namespace ft = ftxui;

class dashboard {
    class timetable_dashboard : timetable {
        static ft::Element get_timeline_widget(std::shared_ptr<std::vector<api::lesson_t>> day);
    public:
        static ft::Component get_timetable_widget(api* api);
    };

    class grades_dashboard {
    public:
        static ft::Component get_grades_widget(api* api);
    };

public:
    static void dashboard_display(ft::Component dashboard_component, api* api);
};
