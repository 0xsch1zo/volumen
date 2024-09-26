#pragma once
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class error {
    bool show;
    ft::Component error_component = ft::Container::Vertical({ft::Button("OK", []{})});

public:
    class volumen_exception : std::runtime_error {
        const char* message;
        const char* FUNCTION;
    public:
        volumen_exception(const char* msg, const char* FUNCTION);
        const char* get_func() const noexcept;
        virtual const char* what() const noexcept override;
    };

    ft::Component handler_component(const char* FUNCTION, const char* what);
    ft::Component get_component();
    bool* get_show();
    void handler(volumen_exception& e);
    void handler(std::exception& e);
};