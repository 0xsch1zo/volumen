#include "error_handler.hpp"

error::volumen_exception::volumen_exception(const char* msg, const char* FUNCTION_) : 
    std::runtime_error(msg),
    FUNCTION(FUNCTION_)
{}

const char* error::volumen_exception::get_func() const { return FUNCTION; }

