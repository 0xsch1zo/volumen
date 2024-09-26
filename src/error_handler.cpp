#include "error_handler.hpp"
#include "utils.hpp"

error::volumen_exception::volumen_exception(const char* msg, const char* FUNCTION_) : 
    std::runtime_error(msg),
    message(msg),
    FUNCTION(FUNCTION_)
{}

const char* error::volumen_exception::get_func() const noexcept { return FUNCTION; }

const char* error::volumen_exception::what() const noexcept { return message; }

ft::Component error::get_component() { return error_component; }

ft::Component error::handler_component(const char* FUNCTION, const char* what) {
    auto ok_button = ft::Button("OK", [&]{ show = true; }, utils::button_rounded());

    return ft::Renderer(ok_button, [=]{
        return ft::vbox({
            ft::text("An exception occured!"),
            ft::separator(),
            ft::text("In:" + std::string(FUNCTION) + ":" + std::string(what))
            | ft::color(ft::Color::Red),
            ok_button->Render()
        }) | ft::borderStyled(ft::Color::Green);
    });
}

void error::handler(error::volumen_exception& e) {
    error_component->Add(handler_component(e.get_func(), e.what()));
    show = true;
}

bool* error::get_show() { return &show; }