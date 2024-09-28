#include "error_handler.hpp"
#include "utils.hpp"

error::volumen_exception::volumen_exception(const std::string& msg, const std::string& FUNCTION_) : 
    std::runtime_error(msg.c_str()),
    message(msg),
    FUNCTION(FUNCTION_)
{}

const char* error::volumen_exception::get_func() const noexcept { return FUNCTION.c_str(); }

const char* error::volumen_exception::what() const noexcept { return message.c_str(); }

ft::Component error::get_component() { return error_component; }

ft::Component error::handler_component(const std::string& FUNCTION, const std::string& what) {
    auto ok_button = ft::Button("OK", [&]{ show = false; }, utils::button_rounded());
    const auto error_msg_size = ft::size(ft::WIDTH, ft::EQUAL, 30);

    return ft::Renderer(ok_button, [=]{
        return ft::vbox({
            ft::text("An exception occured!"),
            ft::separator(),
            ft::paragraph("In: " + FUNCTION + ": " + what)
            | error_msg_size
            | ft::color(ft::Color::Red),
            ok_button->Render()
        }) | ft::borderStyled(ft::Color::Green);
    });
}

ft::Component error::handler_component(const std::string& what) {
    auto ok_button = ft::Button("OK", [&]{ show = false; }, utils::button_rounded());

    return ft::Renderer(ok_button, [=]{
        return ft::vbox({
            ft::text("An exception occured!"),
            ft::separator(),
            ft::text(what)
            | ft::color(ft::Color::Red),
            ok_button->Render()
        }) | ft::borderStyled(ft::Color::Green);
    });
}

void error::handler(error::volumen_exception& e) {
    error_component->DetachAllChildren();
    error_component->Add(handler_component(e.get_func(), e.what()));
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