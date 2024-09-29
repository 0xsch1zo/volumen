#include "dashboard.hpp"
#include <ftxui/component/component.hpp>
#include <memory>

ft::Component dashboard::grades_dashboard::get_grades_widget(api* api) {
    const auto grades_p = api->get_recent_grades();
    auto grade_component = ft::Container::Vertical({});

    for (const auto& grade : *grades_p) {
        grade_component->Add(ft::MenuEntry({
            .label = grade.subject,
            .transform = [=](const ft::EntryState s) {
                auto base = [&](ft::Color color){
		 			return ft::window(
						ft::text(s.label),
                    	ft::text(grade.grade + ", " + grade.category)
						| ft::color(ft::Color::White)
					) | ft::color(color);
				};

                if(s.focused)
                    return base(ft::Color::Green);

                if(s.active)
                    return base(ft::Color::Red);
                
                return base(ft::Color::White);
            }
        }));
    }

    return ft::Renderer(grade_component, [=]{
		const std::string grade_window_name = "Recent grades";

        if(grade_component->Focused())
			return ft::window(
				ft::text(grade_window_name)
				| ft::hcenter,
				grade_component->Render()
				| ft::color(ft::Color::White)
			) | ft::color(ft::Color::Green);
        
        return ft::window(
			ft::text(grade_window_name)
			| ft::hcenter,
			grade_component->Render()
		);
    });
}
