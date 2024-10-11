cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JSON_Diagnostics ON)
add_compile_definitions(VOLUMEN_TESTING)

include(FetchContent)
# Hacky workaround for curlpp
include_directories(./_deps/curlpp-src/include/)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG dfb9558eaff417ee7909169d0abd66e6c52c9d55 
)

# FetchContent_Declare(curlpp
#   GIT_REPOSITORY https://github.com/jpbarrette/curlpp
#   GIT_TAG v0.8.1
# )

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.14.1
)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.3
)

FetchContent_Declare(cpr 
  GIT_REPOSITORY https://github.com/libcpr/cpr
  GIT_TAG 1.10.5
)

FetchContent_MakeAvailable(ftxui spdlog json cpr)

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
)

target_link_libraries(volumen
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE cpr::cpr
  spdlog
)

add_executable(benchmark
  benchmarks/main.cpp
  benchmarks/benchmarks.cpp
  src/auth.cpp
  src/api.cpp 
  src/utils.cpp
  src/error_handler.cpp
)

target_link_libraries(benchmark
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE cpr::cpr
  spdlog
)