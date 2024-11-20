#include <misc/error_handler.hpp>
#include <misc/utils.hpp>
#include <ui/custom_ui.hpp>

constexpr std::string error_type_to_str(error::type t) {
    // Why?
    // Thanks to c++ not having a way to get a name of a variable of enum type
    switch(t) {
        case error::auth_error: return "auth_error";
        case error::request_failed: return "request_failed";
        case error::json_parsing_error: return "json_parsing_erro";
        case error::quit_before_login: return "quit_before_login";
        case error::generic_error: return "generic_error";
        default: return "Unknown error";
    };
}

error::volumen_exception::volumen_exception(const std::string& msg, const std::string& FUNCTION, error::type error_type) : 
    std::runtime_error(msg.c_str()),
    message(msg),
    FUNCTION(FUNCTION),
    e_type(error_type)
{}

const char* error::volumen_exception::get_func() const noexcept { return FUNCTION.c_str(); }

const char* error::volumen_exception::what() const noexcept { return message.c_str(); }

error::type error::volumen_exception::get_type() const noexcept { return e_type; }

const std::string error::volumen_exception::get_error_message() const noexcept { 
    return  "In: " + FUNCTION + ". " +
            "Of type: " + error_type_to_str(e_type) + ". " +
            "With context: " + message;
}

ft::Component error::get_component() const { return error_component; }

// TODO: Addd error types and error messages as variables
ft::Component error::handler_component(const std::string& message) {
    auto ok_button = ft::Button("OK", [&]{ show = false; }, custom_ui::button_rounded());
    const auto error_msg_size = ft::size(ft::WIDTH, ft::EQUAL, 30);

    return ft::Renderer(ok_button, [=, this]{
        return custom_ui::focus_managed_border_box(
            ft::vbox({
                ft::text("An exception occured!"),
                ft::separator(),
                ft::paragraph(message)
                | error_msg_size,
                ok_button->Render()
            }),
            { .active = false, .focused = true }
        );
    });
}

void error::handler(error::volumen_exception& e) {
    error_component->DetachAllChildren();
    error_component->Add(handler_component(e.get_error_message()));
    show = true;
}

void error::handler(std::exception& e) {
    error_component->DetachAllChildren();
    error_component->Add(handler_component(e.what()));
    show = true;
}

bool* error::get_show() { return &show; }

void error::wrapper(std::function<void()> caller_lamda) {
    try{
        caller_lamda();
    }
    catch(error::volumen_exception& ex) {
        handler(ex);
    }
    catch(std::exception& ex) {
        handler(ex);
    }
}