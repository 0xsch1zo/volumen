#pragma once
#include "api.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

namespace ft = ftxui; 

class timetable {
    ft::Component timetable_contents = ft::Container::Vertical({});
    static ft::Component lessons(std::shared_ptr<std::vector<api::lesson_t>> day);
protected:
    static int get_day_of_week(api* api);
public:
    // Public because std::async can't handle default arguments
    enum {
        CURRENT,
        PREV,
        NEXT
    };
    void timetable_display(
        ft::Component timetable_component, 
        api* api, 
        int* selector, 
        std::string url
    );
};