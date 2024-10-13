#include "annoucements.hpp"
#include "custom_ui.hpp"
#include "tab.hpp"
#include "utils.hpp"
#include <thread>
#include <future>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>

ft::Component annoucements::content_view() {
    auto annoucements = annoucements_o.at(selected);
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
    annoucements_o = api->get_annoucments();

    std::vector<api::content_t*> contents;
    contents.reserve(annoucements_o.size());
    for(const auto& annoucement : annoucements_o)
        contents.emplace_back((api::content_t*)&annoucement);

    auto annoucement_components = custom_ui::content_box(contents);

    // Remove loading screen
    content_component->ChildAt(0)->Detach();
    content_component->Add(ft::Renderer(annoucement_components, [=]{ return annoucement_components->Render() | ft::yframe; })
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
