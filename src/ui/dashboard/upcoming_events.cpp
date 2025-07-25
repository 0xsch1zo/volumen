#include <ui/custom_ui.hpp>
#include <ui/dashboard.hpp>

ft::Component dashboard::upcoming_events::get_component(api *api) {
  auto events = api->get_events_unstructured();
  const std::string today = api->get_today();

  // Delete the event if it's date is set to before today
  std::erase_if(events,
                [&](const api::event_t &event) { return event.date <= today; });

  // Sort to get the upcoming events first
  std::sort(events.begin(), events.end(),
            [](const api::event_t &event_a, const api::event_t &event_b) {
              return event_a.date <= event_b.date;
            });

  auto events_component = ft::Container::Vertical({});
  if (events.empty())
    events_component->Add(ft::Renderer([](bool focused) {
      return ft::text("No upcoming events. Peace finally.") | ft::center;
    }));
  for (const auto &event : events) {
    events_component->Add(ft::MenuEntry(
        {.label = event.subject.has_value()
                      ? *event.subject + " | " + event.category
                      : event.category,
         .transform = [=](const ft::EntryState &s) {
           return custom_ui::focus_managed_window(
               ft::text(s.label),
               ft::vbox({
                   ft::text("Date: " + event.date),
                   ft::paragraph("Description: " + event.description),
               }),
               {.active = s.active, .focused = s.focused});
         }}));
  }

  events_component = ft::Renderer(events_component,
                                  [=, this] {
                                    return events_component->Render() |
                                           ft::vscroll_indicator | ft::yframe |
                                           ft::reflect(box_);
                                  }) |
                     switch_focusable_component();
  return custom_ui::custom_component_window(ft::text("Upcoming events"),
                                            events_component);
}
