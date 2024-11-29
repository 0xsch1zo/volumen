#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ui/timetable.hpp>

namespace ft = ftxui;

// Todo add hoverability
class dashboard {
    class dashboard_widget {
    protected:
        ft::Box box_;
    public:
        virtual ft::Component get_component(api* api) = 0;
        std::pair<int, int> get_size() const { 
            return {
                box_.x_max - box_.x_min, 
                box_.y_max - box_.y_min,
            };
        }
    };
    class timetable_dashboard : public dashboard_widget {
        static ft::Element get_timeline_widget(const std::vector<api::lesson_t>& day);
        bool active{};
    public:
        ft::Component get_component(api* api) override;
    };

    class grades_dashboard : public dashboard_widget {
        bool active{};
    public:
        ft::Component get_component(api* api) override;
    };

    class weekend_bar : public dashboard_widget {
    public:
        ft::Component get_component(api* api) override;
    };

    class upcoming_events : public dashboard_widget {
    public:
        ft::Component get_component(api* api) override;
    };

    static ft::Component switch_focusable_component(ft::Component content);
    static ft::ComponentDecorator switch_focusable_component();
public:
    static void dashboard_display(ft::Component dashboard_component, api* api);
};
