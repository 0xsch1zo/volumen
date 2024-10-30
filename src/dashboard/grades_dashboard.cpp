#include "dashboard.hpp"
#include "../custom_ui.hpp"
#include <ftxui/component/component.hpp>
#include <memory>

ft::Component dashboard::grades_dashboard::get_grades_widget(api* api) {
    const auto grades_o = api->get_recent_grades();
    auto grade_component = ft::Container::Vertical({});

    for (const auto& grade : grades_o) {
        grade_component->Add(ft::MenuEntry({
            .label = grade.subject,
            .transform = [=](const ft::EntryState& s) {
                return custom_ui::focus_managed_window(
                    ft::text(s.label), 
                    ft::text(grade.grade + ", " + grade.category), 
                    { .active = s.active, .focused = s.focused }
                );
            }
        }));
    }

    return ft::Renderer(grade_component, [=, this] {
		const std::string grade_window_name = "Recent grades";

        return custom_ui::focus_managed_window(
            ft::text(grade_window_name)
			| ft::hcenter,
			grade_component->Render(), 
            { .active = active, .focused = grade_component->Focused()}
        );
    }) | ft::Hoverable(&active);
}
