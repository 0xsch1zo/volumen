#include <ui/annoucements.hpp>
#include <ui/custom_ui.hpp>
#include <ui/main_ui.hpp>
#include <misc/utils.hpp>
#include <thread>
#include <future>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>

ft::Component annoucements::content_view() {
    auto annoucements = annoucements_.at(selected_);
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
    }) | ft::CatchEvent(utils::exit_active_screen_on_keybind());
}

void annoucements::content_display(
ft::Component content_component,
api* api,
size_t* redirect,
std::mutex* redirect_mutex) {
    annoucements_ = api->get_annoucments();

    std::vector<api::content_t*> contents;
    contents.reserve(annoucements_.size());
    for(const auto& annoucement : annoucements_)
        contents.emplace_back((api::content_t*)&annoucement);

    auto annoucement_components = custom_ui::content_boxes(contents, &selected_);

    // Remove loading screen
    content_component->ChildAt(0)->Detach();
    content_component->Add(ft::Renderer(annoucement_components, [=, this]{ return annoucement_components->Render() | ft::yframe; })
    | ft::CatchEvent([=, this](ft::Event event){
        if(event == ft::Event::Return) {
            auto* screen = ft::ScreenInteractive::Active();
            screen ? screen->Exit() : throw error::volumen_exception(__FUNCTION__, "", error::no_active_screen_error);
            redirect_mutex->lock();
            *redirect = main_ui::ANNOUCEMENT_VIEW; 
            redirect_mutex->unlock();
            return true;
        }
        return false;
    }));
}
