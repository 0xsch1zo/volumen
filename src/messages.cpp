#include "messages.hpp"
#include "utils.hpp"
#include "tab.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

// TODO: make variable for accent color
messages::messages(ft::ScreenInteractive* main_screen) {
    main_screen_p = main_screen;
}

ft::Component messages::content_view() {
    auto message = messages_p->messages->at(selected);
    const std::string deliminator = " | ";
    const std::string quit_message = "Press q or Ctrl+C to quit";
    const int DATE_SIZE = 10;
    char date_s[DATE_SIZE];
    std::time_t t = message.send_date;
    std::tm* date = std::localtime(&t);
    strftime(date_s, sizeof(date), "%Y-%m-%d", date);
    return ft::Renderer([=]{
        return ft::vbox({
            ft::text("Author: " + message.sender + deliminator + "Date:" + date_s),
            ft::separator(),
            ft::separatorEmpty(),
            ft::text(message.subject) | ft::bold,
            ft::separatorEmpty(),
            ft::vbox({utils::split(message.content)}) | ft::yframe,
            ft::filler(),
            ft::separator(),
            ft::text(quit_message)
        });
    });
}

void messages::content_display(
ft::Component content_component,
api* api,
size_t* redirect,
std::mutex* redirect_mutex) {
    messages_p = api->get_messages();

    const size_t PREVIEW_SIZE = 300;

    const std::string deliminator = " | ";
    auto menu_entries = ft::Container::Vertical({});

    
    for(int i{}; i < messages_p->messages->size(); i++) {
        const std::string content = messages_p->messages->at(i).content;
        menu_entries->Add(ft::MenuEntry({
            .label = (content.size() < PREVIEW_SIZE) ? content : content.substr(0, PREVIEW_SIZE) + "...",
            .transform = [=](const ft::EntryState &s) {
                ft::Element entry = ft::paragraph(s.label);
                if(s.focused) {
                    selected = i;
                    entry = ft::window(
                        ft::hbox({
                            ft::text(messages_p->messages->at(i).subject) | ft::bold, 
                            ft::text(deliminator + messages_p->messages->at(i).sender)
                        }), 
                        entry | ft::color(ft::Color::White)
                    ) | ft::color(ft::Color::Green);
                }
                else
                    entry = ft::window(
                        ft::hbox({
                            ft::text(messages_p->messages->at(i).subject) | ft::bold,
                            ft::text(deliminator + messages_p->messages->at(i).sender)
                        }), 
                        entry | ft::color(ft::Color::GrayLight));
                return entry;
            }
        }));
    }
 
    // Remove loading screen
    content_component->ChildAt(0)->Detach();
    content_component->Add(ft::Renderer(menu_entries, [=]{ return menu_entries->Render() | ft::yframe; })
    | ft::CatchEvent([&](ft::Event event){
        if(event == ft::Event::Return) {
            main_screen_p->Exit();
            //std::lock_guard<std::mutex> lock(*redirect_mutex);
            redirect_mutex->lock();
            *redirect = tab::MESSAGE_VIEW; 
            redirect_mutex->unlock();
            return true;
        }
        return false;
    }));
}
