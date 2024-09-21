cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
# Hacky workaround for curlpp
include_directories(./_deps/curlpp-src/include/)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG 535290bb3bce8f2bec9557e9d14bb92be3d7b6ab
)

FetchContent_Declare(curlpp
  GIT_REPOSITORY https://github.com/jpbarrette/curlpp
  GIT_TAG v0.8.1
)

FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.14.1
)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(ftxui curlpp spdlog json)

project(volumen)
add_executable(volumen
  src/main.cpp
  src/login.cpp
  src/authorization.cpp
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
)

target_link_libraries(volumen
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
  curlpp
  spdlog
)
