#include <ftxui/component/component.hpp>

namespace ft = ftxui;

class error {
public:
    class volumen_exception : std::runtime_error {
        const char* FUNCTION;
    public:
        volumen_exception(const char* msg, const char* FUNCTION);
        const char* get_func() const;
    };

    ft::Component handler_component();
    void handler(volumen_exception& e);
    void handler(std::exception& e);
};