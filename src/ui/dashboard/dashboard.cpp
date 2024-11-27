#include <ui/dashboard.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

ft::Component dashboard::switch_focusable_component(ft::Component content) {
    class Impl : public ft::ComponentBase {
        bool enabled_{};
        ft::Box box_;
    public:
        Impl(ft::Component content) {
            Add(content);
        }

        ft::Element Render() override {
             return children_[0]->Render()
             | (enabled_ ? ft::nothing : ft::dim)
             | ft::reflect(box_);
        }

        bool OnEvent(ft::Event event) override {
            if(event == ft::Event::Return) {
                enabled_ = !enabled_;
                return true;
            }

            if(event.is_mouse() && !enabled_ && 
                box_.Contain(event.mouse().x, event.mouse().y)) {
                enabled_ = true;
                return true;
            }

            if(event.is_mouse() && enabled_ && 
                !box_.Contain(event.mouse().x, event.mouse().y)) {
                enabled_ = false;
                return true;
            }

            if(children_[0]->OnEvent(event))
                return true;

            return false;
        };

        bool Focusable() const override { return enabled_; };
    };

    class Focusable_Encapsulation : public ft::ComponentBase {
        public:
        Focusable_Encapsulation (ft::Component content) {
            Add(content);
        }

        bool Focusable() const override { return true; }
    };

    return ft::Make<Focusable_Encapsulation>(std::move(ft::Make<Impl>(std::move(content))));
}

ft::ComponentDecorator dashboard::switch_focusable_component() {
    return [](ft::Component content) {
        return switch_focusable_component(std::move(content));
    };
}

void dashboard::dashboard_display(ft::Component dashboard_component, api* api) {
    grades_dashboard grades_dashboard_o;
    timetable_dashboard timetable_dashboard_o;

    auto flex = [](ft::Component component) {
        return ft::Renderer(component, [=]{
            return component->Render()
            | ft::flex;
        });
    };
    auto grades = flex(grades_dashboard_o.get_grades_widget(api));
    auto timetable = flex(timetable_dashboard_o.get_timetable_widget(api));
    auto events = flex(upcoming_events::get_upcoming_events(api));
    auto weekend_bar = flex(weekend_bar::get_weekend_bar(api));
    
    auto dashboard_components = ft::Container::Vertical({
        ft::Container::Horizontal({
            ft::Container::Vertical({
                grades,
                timetable
            }),
            events
        }),
        ft::Container::Horizontal({
            weekend_bar
        })
    });

    // Remove loading screen
    dashboard_component->DetachAllChildren();
    dashboard_component->Add(dashboard_components);
}
