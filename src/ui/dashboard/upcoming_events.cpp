#include <ui/dashboard.hpp>
#include <ui/custom_ui.hpp>

ft::Component dashboard::upcoming_events::get_upcoming_events(api* api) {
    auto events = api->get_events_unstructured();
    const int max_event_count = 4;

    auto events_component = ft::Container::Vertical({});
    int i{};
    for(const auto& event : events) {
        if(i++ >= max_event_count)
            break;
        events_component->Add(ft::MenuEntry({
            .label = event.subject + " | " + event.category,
            .transform = [=](const ft::EntryState& s) {
                return custom_ui::focus_managed_window(ft::text(s.label), ft::vbox({
                        ft::text("Date: " + event.date),
                        ft::paragraph("Description: " + event.description),
                    }),
                    { .active = s.active, .focused = s.focused }
                );
            }
        }));
    }

    events_component = custom_ui::custom_component_window(ft::text("Upcoming events"), events_component);
    return events_component;
}