#include <auth/auth.hpp>
#include <misc/config.hpp>
#include <misc/ssave.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <ui/custom_ui.hpp>
#include <ui/login.hpp>
#include <ui/main_ui.hpp>
#include <volumen.hpp>

namespace volumen {

void volumen() {
  auto logger = spdlog::stderr_color_mt("stderr");
  logger->enable_backtrace(32);
  spdlog::set_default_logger(logger);

  config *config_p;

  try {
    config_p = new config();
  } catch (std::exception &e) {
    spdlog::error(e.what());
    exit(1);
  }

  custom_ui::init(config_p);
  login login(config_p);
  auth auth_o([&] {
    try {
      login.login_screen(auth_o);
    } catch (error::volumen_exception &e) {
      if (e.get_type() == error::auth_error)
        spdlog::error(e.get_error_message());

      delete config_p;
      exit(0);
    } catch (std::exception &e) {
      spdlog::error(e.what());
    }
  });

  main_ui main_ui(config_p, auth_o);
  do {
    auth_o.refresh_api_tokens();
    if (!ssave::exists(auth::login_service_field))
      login.choose_account_screen(auth_o);
  } while (ssave::exists(auth::login_service_field) &&
           main_ui.display_interface(ssave::get(auth::login_service_field)));

  delete config_p;
  spdlog::dump_backtrace();
}

} // namespace volumen
