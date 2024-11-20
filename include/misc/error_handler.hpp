#pragma once
#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class error {
    bool show{};
    ft::Component error_component = ft::Container::Vertical({});

public:
    enum type {
        auth_error,
        request_failed,
        json_parsing_error,
        quit_before_login,
        generic_error
    };

    class volumen_exception : std::runtime_error {
        const std::string message;
        const std::string FUNCTION;
        const error::type e_type;
    public:
        volumen_exception(const std::string& FUNCTION, const std::string& msg, error::type error_type = generic_error);
        const char* get_func() const noexcept;
        virtual const char* what() const noexcept override;
        error::type get_type() const noexcept;
        const std::string get_error_message() const noexcept;
    };

    ft::Component handler_component(const std::string& message);
    ft::Component get_component() const ;
    bool* get_show();
    void handler(volumen_exception& e);
    void handler(std::exception& e);
    void wrapper(std::function<void()> caller_lambda);
};