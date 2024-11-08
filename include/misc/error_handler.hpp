#pragma once
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class error {
    bool show{};
    ft::Component error_component = ft::Container::Vertical({});

public:
    class volumen_exception : std::runtime_error {
        const std::string message;
        const std::string FUNCTION;
    public:
        volumen_exception(const std::string& msg, const std::string& FUNCTION);
        const char* get_func() const noexcept;
        virtual const char* what() const noexcept override;
    };

    ft::Component handler_component(const std::string& FUNCTION, const std::string& what);
    ft::Component handler_component(const std::string& what);
    ft::Component get_component();
    bool* get_show();
    void handler(volumen_exception& e);
    void handler(std::exception& e);
    void wrapper(std::function<void()> caller_lambda);
};