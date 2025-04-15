#include <api/api.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <misc/utils.hpp>
#include <spdlog/spdlog.h>
#include <ui/custom_ui.hpp>
#include <ui/dashboard.hpp>
#include <ui/timetable.hpp>

// TODO: implement caching
ft::Component dashboard::timetable_dashboard::get_component(api *api) {
  const std::string weekend_prompt = "No lessons today!";
  const api::timetable_t timetable_o = api->get_timetable();
  int tomorrow = (utils::get_day_of_week(api->get_today()) + 1) % 7;
  static int selector{};

  auto timetable_widget = ft::Container::Vertical({});

  int empty_counter{};
  for (const auto &lesson : timetable_o.timetable[tomorrow]) {
    if (lesson.is_empty) {
      empty_counter++;
      continue;
    }
    timetable_widget->Add(ft::MenuEntry(lesson.subject));
  }

  if (empty_counter == timetable_o.timetable[tomorrow].size())
    timetable_widget->Add(ft::MenuEntry(weekend_prompt));

  timetable_widget |= switch_focusable_component();

  auto timeline_widget =
      timetable_dashboard::get_timeline_widget(timetable_o.timetable[tomorrow]);

  auto timetable_container =
      ft::Renderer(timetable_widget,
                   [=, this] {
                     return ft::hbox(
                         {timeline_widget, ft::separator(),
                          timetable_widget->Render() |
                              ft::size(ft::HEIGHT, ft::GREATER_THAN,
                                       timetable_o.timetable[tomorrow].size() -
                                           empty_counter + 2) |
                              ft::reflect(box_)});
                   }) |
      switch_focusable_component();

  return custom_ui::custom_component_window(
      ft::text("Timetable for the next day"), timetable_container);
}

ft::Element dashboard::timetable_dashboard::get_timeline_widget(
    const std::vector<api::lesson_t> &day) {
  const std::string deliminator = " - ";
  const std::string empty_placeholder = "-- - --";
  std::vector<ft::Element> entries;
  ft::Element el;

  int empty_counter{};

  for (const auto &lesson : day) {
    if (lesson.is_empty) {
      empty_counter++;
      continue;
    }
    entries.push_back(ft::text(lesson.start + deliminator + lesson.end));
  }

  if (empty_counter == day.size())
    return ft::text(empty_placeholder);

  return ft::vbox({entries});
}
