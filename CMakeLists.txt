cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JSON_Diagnostics ON)
add_compile_definitions(VOLUMEN_TESTING TOML_ENABLE_UNRELEASED_FEATURES TOML_LANG_UNRELEASED TOML_UNRELEASED_FEATURES)

include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG        1d40687a403b7babdafdd577d10e001840b0f12
)

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG        v1.14.1
)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG        v3.11.3
)

FetchContent_Declare(cpr 
  GIT_REPOSITORY https://github.com/libcpr/cpr
  GIT_TAG         1.10.5
)

FetchContent_Declare(tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus
  GIT_TAG        v3.4.0
)

FetchContent_Declare(keychain
  GIT_REPOSITORY https://github.com/hrantzsch/keychain
  GIT_TAG        82acee89623d0d84e1575147fb83ed387646bce8 
)

FetchContent_MakeAvailable(ftxui spdlog json cpr tomlplusplus keychain)

project(volumen)

add_executable(volumen
  src/main.cpp
  src/login.cpp
  src/dashboard/dashboard.cpp
  src/dashboard/timetable_dashboard.cpp 
  src/dashboard/grades_dashboard.cpp
  src/dashboard/weekend_bar.cpp
  src/utils.cpp 
  src/messages.cpp 
  src/tab.cpp 
  src/api.cpp 
  src/annoucements.cpp 
  src/timetable.cpp
  src/grades.cpp
  src/error_handler.cpp
  src/auth.cpp
  src/custom_ui.cpp
  src/config.cpp
  src/ssave.cpp
)

target_link_libraries(volumen
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE tomlplusplus::tomlplusplus
  PRIVATE cpr::cpr
  PRIVATE keychain 
  PRIVATE spdlog
)

add_executable(benchmark
  benchmarks/main.cpp
  benchmarks/benchmarks.cpp
  src/auth.cpp
  src/api.cpp 
  src/utils.cpp
  src/error_handler.cpp
  src/custom_ui.cpp
  src/config.cpp
  src/ssave.cpp
)

target_link_libraries(benchmark
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE tomlplusplus::tomlplusplus
  PRIVATE cpr::cpr
  PRIVATE keychain 
  PRIVATE spdlog
)
