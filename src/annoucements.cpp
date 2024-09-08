#include "annoucements.hpp"
#include "tab.hpp"
#include "utils.hpp"
#include <thread>
#include <future>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>

// takes in main screen as pointer and content type enum
annoucements::annoucements(ft::ScreenInteractive* main_screen) {
    main_screen_p = main_screen;
}

ft::Component annoucements::content_view() {
    auto annoucements = annoucements_p->at(selected);
    const std::string deliminator = " | ";
    const std::string quit_message = "Press q or Ctrl+C to quit";
    return ft::Renderer([=]{ 
        return ft::vbox({
            ft::text("Author: " + annoucements.author + deliminator + "Start date: " + annoucements.start_date + deliminator + "End date: " + annoucements.end_date),
            ft::separator(),
            ft::separatorEmpty(),
            ft::text(annoucements.subject) | ft::bold,
            ft::separatorEmpty(),
            ft::vbox({utils::split(annoucements.content)}) | ft::yframe,
            ft::filler(),
            ft::separator(),
            ft::text(quit_message)
        });
    });
}

void annoucements::content_display(
ft::Component content_component,
api* api,
size_t* redirect,
std::mutex* redirect_mutex) {
    annoucements_p= api->get_events();

    const size_t PREVIEW_SIZE = 300;

    const std::string deliminator = " | ";
    auto menu_entries = ft::Container::Vertical({});

    
    for(int i{}; i < annoucements_p->size(); i++) {
        menu_entries->Add(ft::MenuEntry({
            .label = annoucements_p->at(i).content.substr(0, PREVIEW_SIZE) + "...",
            .transform = [=](const ft::EntryState &s) {
                ft::Element entry = ft::paragraph(s.label);
                if(s.focused) {
                    selected = i;
                    entry = ft::window(
                        ft::hbox({
                            ft::text(annoucements_p->at(i).subject) | ft::bold, 
                            ft::text(deliminator + annoucements_p->at(i).author)
                        }), 
                        entry | ft::color(ft::Color::White)
                    ) | ft::color(ft::Color::Green);
                }
                else
                    entry = ft::window(
                        ft::hbox({
                            ft::text(annoucements_p->at(i).subject) | ft::bold,
                            ft::text(deliminator + annoucements_p->at(i).author)
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
            *redirect = tab::ANNOUCEMENT_VIEW; 
            redirect_mutex->unlock();
            return true;
        }
        return false;
    }));
}
