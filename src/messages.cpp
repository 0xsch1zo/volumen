#include "messages.hpp"
#include "custom_ui.hpp"
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

ft::Component messages::content_view() {
    auto message = messages_.at(selected_);
    const std::string deliminator = " | ";
    const std::string quit_message = "Press q or Ctrl+C to quit";
    const int DATE_SIZE = 10;
    auto date_timepoint = std::chrono::sys_seconds(std::chrono::seconds(message.send_date));
    std::string date = std::format("{:%Y-%m-%d}", date_timepoint);
    return ft::Renderer([=, this]{
        return ft::vbox({
            ft::text("Author: " + message.author + deliminator + "Date: " + date),
            ft::separator(),
            ft::separatorEmpty(),
            ft::text(message.subject) | ft::bold,
            ft::separatorEmpty(),
            ft::vbox({utils::split(message.content)}) | ft::yframe,
            ft::filler(),
            ft::separator(),
            ft::text(quit_message)
        });
    }) | ft::CatchEvent(utils::exit_on_keybind(screen_exit_));
}

void messages::content_display(
ft::Component content_component,
api* api,
size_t* redirect,
std::mutex* redirect_mutex) {
    messages_ = api->get_messages();
    std::vector<api::content_t*> contents;
    contents.reserve(messages_.size());
    for(const auto& message : messages_)
        contents.emplace_back((api::content_t*)&message);

    auto message_components = custom_ui::content_boxes(contents, &selected_);
 
    // Remove loading screen
    content_component->ChildAt(0)->Detach();
    content_component->Add(ft::Renderer(message_components, [=, this]{ return message_components->Render() | ft::yframe; })
    | ft::CatchEvent([=, this](ft::Event event){
        if(event == ft::Event::Return) {
            screen_exit_();
            redirect_mutex->lock();
            *redirect = tab::MESSAGE_VIEW; 
            redirect_mutex->unlock();
            return true;
        }
        return false;
    }));
}
