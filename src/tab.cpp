#include "tab.hpp"
#include "dashboard/dashboard.hpp"
#include "messages.hpp"
#include "annoucements.hpp"
#include "authorization.hpp"
#include "timetable.hpp"
#include "content.hpp"
#include "grades.hpp"
#include "error_handler.hpp"
#include <future>
#include <chrono>
#include <bitset>
#include <memory>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>

void tab::tab_error_wrapper(
    error* e, 
    std::function<void()> caller_lambda,  
    ft::Component tab_container, 
    tab::tabs_t tabs
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
void tab::display_interface(int synergia_account_i) {
using namespace std::chrono_literals;
    auto main_screen = ft::ScreenInteractive::FullscreenAlternateScreen();
    
    error e;
    api api(authorization::get_synergia_accounts().at(synergia_account_i));
    std::unique_ptr<content> annoucements_p = std::make_unique<annoucements>(&main_screen);
    std::unique_ptr<content> messages_p = std::make_unique<messages>(&main_screen);
    timetable t;
    grades g;

    int tab_selected{};
    std::vector<std::string> menu = {
        "Dashboard",
        "Messages",
        "Events",
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
    &tab_selected) | ft::Modal(e.get_component(), e.get_show());
    
    ft::Component container = ft::Container::Vertical({
        tab_menu,
        tab_container
    });

    size_t redirect{EXIT};
    std::mutex redirect_mutex;

    const size_t TO_LAZY_LOAD = 5;
    const auto ANIMATION_WAIT = 70ms;
    std::bitset<TO_LAZY_LOAD> envoked_lazy_load{};

    // Waits for all the threads for loading content to finish.
    // While they're working increment counter for loading spinner
    // And update the screen    

    auto loading_animation = std::async(std::launch::async, ([&]{ 
        // Run until all things have been loaded
        while(!envoked_lazy_load.all()
        || dashboard_load_handle.wait_for(0ms)      != std::future_status::ready
        || messages_load_handle.wait_for(0ms)       != std::future_status::ready
        || annoucements_load_handle.wait_for(0ms)   != std::future_status::ready
        || timetable_load_handle.wait_for(0ms)      != std::future_status::ready
        || grades_load_handle.wait_for(0ms)         != std::future_status::ready) {
            std::this_thread::sleep_for(ANIMATION_WAIT);
            if(load_state >= 8)
                load_state = 0;
            load_state++;
            main_screen.PostEvent(ft::Event::Special("Spaghetti code"));
        }
    }));

// Prevent std::async ftom being called twice for the same handle
#define GUARD(x) if(envoked_lazy_load[x]) break

    auto renderer = ft::Renderer(container, [&]{
        // These will get lazy loaded
        // Gets executed on first launch of a component and reload
        if(!envoked_lazy_load.all()) {
            switch(tab_selected) {
                case DASHBOARD:
                    GUARD(0);

                    dashboard_load_handle = std::async(std::launch::async, tab_error_wrapper, &e, [&]{
                        dashboard::dashboard_display(dashboard_component, &api); 
                    }, tab_container, DASHBOARD);

                    envoked_lazy_load[0] = true;
                    break;

                case MESSAGES:
                    GUARD(1);

                    messages_load_handle = std::async(std::launch::async, tab_error_wrapper, &e, [&]{
                        messages_p->content_display(messages_component, &api, &redirect, &redirect_mutex); 
                    }, tab_container, MESSAGES);

                    envoked_lazy_load[1] = true;
                    break;

                case ANNOUCEMENTS: 
                    GUARD(2);

                    annoucements_load_handle = std::async(std::launch::async, tab_error_wrapper, &e, [&]{
                        annoucements_p->content_display(annoucements_component, &api, &redirect, &redirect_mutex); 
                    }, tab_container, ANNOUCEMENTS);

                    envoked_lazy_load[2] = true;
                    break;

                case TIMETABLE:
                    GUARD(3);

                    timetable_load_handle = std::async(std::launch::async, tab_error_wrapper, &e, [&]{
                        t.timetable_display(timetable_component, &api, &selsd, nullptr);
                    }, tab_container, TIMETABLE);

                    envoked_lazy_load[3] = true;
                    break;
                
                case GRADES:
                    GUARD(4);

                    grades_load_handle = std::async(std::launch::async, tab_error_wrapper, &e, [&]{
                        g.grades_display(grades_component, &api);
                    }, tab_container, GRADES);

                    envoked_lazy_load[4] = true;
                    break;
            }
        }

        return ft::vbox({
            tab_menu->Render(),
            tab_container->Render()
        });
    });

main_loop:
    main_screen.Loop(renderer);
    switch(redirect) {
        case EXIT:
            break;

        case MESSAGE_VIEW:
            main_screen.Loop(messages_p->content_view());
            redirect = EXIT;
            goto main_loop;
            break;

        case ANNOUCEMENT_VIEW:
            main_screen.Loop(annoucements_p->content_view());
            redirect = EXIT;
            goto main_loop;
            break;
    }
}