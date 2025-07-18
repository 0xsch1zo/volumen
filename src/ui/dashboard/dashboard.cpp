#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <spdlog/spdlog.h>
#include <ui/custom_ui.hpp>
#include <ui/dashboard.hpp>
#include <ui/main_ui.hpp>

ft::Component dashboard::switch_focusable_component(ft::Component content) {
  class Impl : public ft::ComponentBase {
    bool enabled_{};
    bool hovered_{};
    ft::Box box_;

  public:
    Impl(ft::Component content) { Add(content |= ft::Hoverable(&hovered_)); }

    ft::Element Render() {
      if (!Parent())
        throw error::volumen_exception(__FUNCTION__, "No parent found",
                                       error::generic_error);

      // Deactivate when not hovered or focused
      if (enabled_ && (!Parent()->Focused() && !hovered_))
        enabled_ = false;

      // Activate if hovered and not focused
      if (!enabled_ && (hovered_ && !Parent()->Focused()))
        enabled_ = true;

      return children_[0]->Render() | (enabled_ ? ft::nothing : ft::dim) |
             ft::reflect(box_);
    }

    bool OnEvent(ft::Event event) override {
      if (event == ft::Event::Return) {
        enabled_ = !enabled_;
        return true;
      }

      if (children_[0]->OnEvent(event))
        return true;

      return false;
    };

    bool Focusable() const override { return enabled_; };
  };

  class Focusable_Encapsulation : public ft::ComponentBase {
  public:
    Focusable_Encapsulation(ft::Component content) { Add(content); }

    bool Focusable() const override { return true; }
  };

  return ft::Make<Focusable_Encapsulation>(
      std::move(ft::Make<Impl>(std::move(content))));
}

ft::ComponentDecorator dashboard::switch_focusable_component() {
  return [](ft::Component content) {
    return switch_focusable_component(std::move(content));
  };
}

void dashboard::dashboard_display(ft::Component dashboard_component, api *api) {
  grades_dashboard grades_dashboard_o;
  timetable_dashboard timetable_dashboard_o;
  upcoming_events upcoming_events_o;
  weekend_bar weekend_bar_o;

  auto xflex_component = [](ft::Component component) {
    return ft::Renderer(component,
                        [=] { return component->Render() | ft::xflex; });
  };

  auto grades = grades_dashboard_o.get_component(api) | xflex_component;
  auto timetable = timetable_dashboard_o.get_component(api) | xflex_component;
  auto events = upcoming_events_o.get_component(api) | xflex_component;
  auto weekend_bar = weekend_bar_o.get_component(api) | xflex_component;

  auto dashboard_components = ft::Container::Vertical(
      {ft::Container::Horizontal(
           {ft::Container::Vertical(
                {ft::Renderer(
                     grades,
                     [grades, &timetable_dashboard_o, &weekend_bar_o] {
                       return grades->Render() |
                              ft::size(
                                  ft::HEIGHT, ft::LESS_THAN,
                                  custom_ui::terminal_height() -
                                      main_ui::top_menu_size -
                                      weekend_bar_o.get_size().second -
                                      timetable_dashboard_o.get_size().second);
                     }),
                 timetable}),
            ft::Renderer(events,
                         [events, &weekend_bar_o] {
                           return events->Render() |
                                  ft::size(ft::HEIGHT, ft::LESS_THAN,
                                           custom_ui::terminal_height() -
                                               main_ui::top_menu_size -
                                               weekend_bar_o.get_size().second);
                         })}),
       ft::Container::Horizontal({weekend_bar})});

  // Remove loading screen
  dashboard_component->DetachAllChildren();
  dashboard_component->Add(dashboard_components);
}
