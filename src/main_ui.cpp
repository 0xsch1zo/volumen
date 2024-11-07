#include "main_ui.hpp"
#include "dashboard/dashboard.hpp"
#include "messages.hpp"
#include "annoucements.hpp"
#include "timetable.hpp"
#include "grades.hpp"
#include "error_handler.hpp"
#include "utils.hpp"
#include <future>
#include <chrono>
#include <bitset>
#include <memory>
#include <thread>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

void main_ui::ui_error_wrapper(
    error* e, 
    std::function<void()> caller_lambda,  
    ft::Component tab_container, 
    main_ui::tabs_t tabs
) {
    try {
        caller_lambda();
    } 
    catch(error::volumen_exception& ex) {
        e->handler(ex);
        // ChildAt(0) Because it's not only container but also modal
        tab_container->ChildAt(0)->ChildAt(0)->ChildAt(tabs)->DetachAllChildren();
    }
    catch(std::exception& ex) {
        e->handler(ex);
        // ChildAt(0) Because it's not only container but also modal
        tab_container->ChildAt(0)->ChildAt(0)->ChildAt(tabs)->DetachAllChildren();
    }
    
}

// TODO: remove unnecessary includes
void main_ui::display_interface(const auth& auth_o, const std::string& picked_login) {
using namespace std::chrono_literals;
    auto main_screen = ft::ScreenInteractive::Fullscreen();
    
    error e;
    api api(auth_o, picked_login);
    annoucements annoucements_o(main_screen.ExitLoopClosure());
    messages messages_o(main_screen.ExitLoopClosure());
    timetable timetable_o(config_p);
    grades grades_o;

    int tab_selected{};
    std::vector<std::string> menu = {
        "Dashboard",
        "Messages",
        "Annoucements",
        "Timetable",
        "Grades"
    };


    const size_t SPINNER_TYPE = 20;    
    size_t load_state{};
    auto loading_screen = [&]{ 
        return ft::Renderer([&load_state]{ return ft::spinner(SPINNER_TYPE, load_state) | ft::center; });
    };

    std::future<void> dashboard_load_handle;
    std::future<void> messages_load_handle;
    std::future<void> annoucements_load_handle;
    std::future<void> timetable_load_handle;
    std::future<void> grades_load_handle;

    int selsd{};
    ft::Component dashboard_component       = ft::Container::Vertical({ loading_screen() });
    ft::Component messages_component        = ft::Container::Vertical({ loading_screen() });
    ft::Component annoucements_component    = ft::Container::Vertical({ loading_screen() });
    ft::Component timetable_component       = ft::Container::Vertical({ loading_screen() });
    ft::Component grades_component          = ft::Container::Vertical({ loading_screen() });

    ft::Component tab_menu = ft::Menu(&menu, &tab_selected, ft::MenuOption::HorizontalAnimated());
    ft::Component tab_container = ft::Container::Tab({
        dashboard_component,
        messages_component,
        annoucements_component,
        timetable_component,
        grades_component
    }, 
    &tab_selected)
    | ft::Modal(e.get_component(), e.get_show());

    ft::Component container = ft::Container::Vertical({
        tab_menu,
        tab_container
    }) | ft::CatchEvent(utils::exit_on_keybind(main_screen.ExitLoopClosure()));

    size_t redirect{EXIT};
    std::mutex redirect_mutex;

    const size_t TO_LAZY_LOAD = 5;
    const auto ANIMATION_WAIT = 70ms;
    std::bitset<TO_LAZY_LOAD> envoked_lazy_load{};

    // Waits for all the threads for loading content to finish.
    // While they're working increment counter for loading spinner
    // And update the screen    

    auto loading_animation = std::jthread([&](std::stop_token s_token){
        // Run until all things have been loaded
        while((!envoked_lazy_load.all()
        || dashboard_load_handle.wait_for(0ms)      != std::future_status::ready
        || messages_load_handle.wait_for(0ms)       != std::future_status::ready
        || annoucements_load_handle.wait_for(0ms)   != std::future_status::ready
        || timetable_load_handle.wait_for(0ms)      != std::future_status::ready
        || grades_load_handle.wait_for(0ms)         != std::future_status::ready
        ) && !s_token.stop_requested()) {
            std::this_thread::sleep_for(ANIMATION_WAIT);
            if(load_state >= 8)
                load_state = 0;
            load_state++;
            main_screen.PostEvent(ft::Event::Special("Spaghetti code"));
        }
    });

// Prevent std::async ftom being called twice for the same handle
#define GUARD(x) if(envoked_lazy_load[x]) break

    auto renderer = ft::Renderer(container, [&]{
        // These will get lazy loaded
        // Gets executed on first launch of a component and reload
        if(!envoked_lazy_load.all()) {
            switch(tab_selected) {
                case DASHBOARD:
                    GUARD(DASHBOARD);

                    dashboard_load_handle = std::async(std::launch::async, ui_error_wrapper, &e, [&]{
                        dashboard::dashboard_display(dashboard_component, &api); 
                    }, tab_container, DASHBOARD);

                    envoked_lazy_load[DASHBOARD] = true;
                    break;

                case MESSAGES:
                    GUARD(MESSAGES);

                    messages_load_handle = std::async(std::launch::async, ui_error_wrapper, &e, [&]{
                        messages_o.content_display(messages_component, &api, &redirect, &redirect_mutex); 
                    }, tab_container, MESSAGES);

                    envoked_lazy_load[MESSAGES] = true;
                    break;

                case ANNOUCEMENTS: 
                    GUARD(ANNOUCEMENTS);

                    annoucements_load_handle = std::async(std::launch::async, ui_error_wrapper, &e, [&]{
                        annoucements_o.content_display(annoucements_component, &api, &redirect, &redirect_mutex); 
                    }, tab_container, ANNOUCEMENTS);

                    envoked_lazy_load[ANNOUCEMENTS] = true;
                    break;

                case TIMETABLE:
                    GUARD(TIMETABLE);

                    timetable_load_handle = std::async(std::launch::async, ui_error_wrapper, &e, [&]{
                        timetable_o.timetable_display(timetable_component, &api, &selsd, "", &main_screen);
                    }, tab_container, TIMETABLE);

                    envoked_lazy_load[TIMETABLE] = true;
                    break;
                
                case GRADES:
                    GUARD(GRADES);

                    grades_load_handle = std::async(std::launch::async, ui_error_wrapper, &e, [&]{
                        grades_o.grades_display(grades_component, &api);
                    }, tab_container, GRADES);

                    envoked_lazy_load[GRADES] = true;
                    break;
            }
        }

        return container->Render();
    });

main_loop:
    main_screen.Loop(renderer);
    switch(redirect) {
        case EXIT:
            break;

        case MESSAGE_VIEW:
            main_screen.Loop(messages_o.content_view());
            redirect = EXIT;
            goto main_loop;
            break;

        case ANNOUCEMENT_VIEW:
            main_screen.Loop(annoucements_o.content_view());
            redirect = EXIT;
            goto main_loop;
            break;
    }

    loading_animation.request_stop();
}