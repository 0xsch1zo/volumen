#include "login.hpp"
#include "authorization.hpp"
#include "api.hpp"
#include "tab.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

// TODO: make loggger  display messages only in backtrace
int main () {
    auto logger = spdlog::stderr_color_mt("stderr");
    logger->enable_backtrace(32); 
    spdlog::set_default_logger(logger);
    login::login_screen();
    //tab::display_interface();
    /*std::string mail;
    std::string pass;
    std::cin >> mail >> pass;
    bool arewefucked;
    authorization::authorize(mail, pass);
    std::vector<authorization::synergia_account_t> accounts = authorization::get_synergia_accounts();
    api api(authorization::get_synergia_accounts()[0]);
    api.get_messages();*/
    //api::get_subjects(authorization::get_synergia_accounts()[0], request);
    spdlog::dump_backtrace();
    cl::Cleanup();
  return 0;
}