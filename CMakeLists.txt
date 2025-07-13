cmake_minimum_required(VERSION 3.25)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JSON_Diagnostics ON)
# Ihnored on windows
file(RELATIVE_PATH relative_to_libs
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
)
set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/${relative_to_libs})

if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY  https://github.com/ArthurSonzogni/ftxui
  GIT_TAG         v6.1.9
)

FetchContent_Declare(spdlog
  GIT_REPOSITORY  https://github.com/gabime/spdlog
  GIT_TAG         v1.15.3
)

FetchContent_Declare(json
  GIT_REPOSITORY  https://github.com/nlohmann/json
  GIT_TAG         v3.12.0
)

FetchContent_Declare(cpr
  GIT_REPOSITORY  https://github.com/libcpr/cpr
  GIT_TAG         1.12.0
)

FetchContent_Declare(tomlplusplus
  GIT_REPOSITORY  https://github.com/marzer/tomlplusplus
  GIT_TAG         v3.4.0
)

FetchContent_Declare(keychain
  GIT_REPOSITORY  https://github.com/hrantzsch/keychain
  GIT_TAG         v1.3.1
)

FetchContent_MakeAvailable(ftxui spdlog json tomlplusplus keychain cpr)

project(volumen VERSION 0.1.8)

add_executable(volumen
  src/main.cpp
  src/volumen.cpp
  src/ui/login.cpp
  src/ui/dashboard/dashboard.cpp
  src/ui/dashboard/timetable_dashboard.cpp
  src/ui/dashboard/grades_dashboard.cpp
  src/ui/dashboard/weekend_bar.cpp
  src/ui/dashboard/upcoming_events.cpp
  src/ui/annoucements.cpp
  src/ui/timetable.cpp
  src/ui/grades.cpp
  src/ui/messages.cpp
  src/ui/main_ui.cpp
  src/ui/custom_ui.cpp
  src/auth/auth.cpp
  src/api/api.cpp
  src/misc/utils.cpp
  src/misc/error_handler.cpp
  src/misc/config.cpp
  src/misc/ssave.cpp
)

target_include_directories(volumen
  PRIVATE include/
)

target_link_libraries(volumen
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE tomlplusplus::tomlplusplus
  PRIVATE cpr::cpr
  PRIVATE keychain
  PRIVATE spdlog::spdlog_header_only
)

install(TARGETS volumen
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)


if(CMAKE_BUILD_TYPE=Debug)
    set(JSON_Diagnostics ON)
    add_compile_definitions(VOLUMEN_TESTING)
    add_executable(benchmark
    benchmarks/main.cpp
    benchmarks/benchmarks.cpp
    src/ui/custom_ui.cpp
    src/auth/auth.cpp
    src/api/api.cpp
    src/misc/utils.cpp
    src/misc/error_handler.cpp
    src/misc/config.cpp
    src/misc/ssave.cpp
  )

    target_include_directories(benchmark
    PRIVATE include/
  )

    target_link_libraries(benchmark
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE tomlplusplus::tomlplusplus
    PRIVATE cpr::cpr
    PRIVATE keychain
    PRIVATE spdlog::spdlog_header_only
  )
endif()
