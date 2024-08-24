#include "messages.hpp"
#include <string>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

// TODO: make it scrollable
// TODO: make variable for accent color
ft::Component messages::message_box(std::vector<message_t>& messages, int& message_chosen, std::function<void()> on_enter) {
    std::vector<std::string> asd;
    const std::string deliminator = " | ";
    std::vector<ft::Component> menu_entries;
    for(auto message : messages) {
        menu_entries.push_back(ft::MenuEntry({
            .label = message.content,
            .transform = [=](const ft::EntryState &s) {
                ft::Element entry = ft::paragraph(s.label);
                if(s.focused)
                    entry = ft::window(ft::hbox({ft::text(message.subject) | ft::bold, ft::text(deliminator + message.author)}), entry, ft::Color::Green);
                else
                    entry = ft::window(ft::hbox({ft::text(message.subject) | ft::bold, ft::text(deliminator + message.author)}), entry);
                return entry;
            }
        }));
    }
    auto menu = ft::Container::Vertical(menu_entries, &message_chosen);
    menu |= ft::CatchEvent([&](ft::Event event){
        if(event == ft::Event::Return) { on_enter(); return true; }
        return false;
    });
    return std::move(menu);
}
