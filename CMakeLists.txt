cmake_minimum_required(VERSION 3.21)
project(LinWin VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO_UNIX REQUIRED IMPORTED_TARGET gio-unix-2.0)

qt_add_executable(linwin
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
    src/app_launcher_widget.cpp
    src/app_launcher_widget.h
    src/clipboard_history_widget.cpp
    src/clipboard_history_widget.h
    src/file_manager_widget.cpp
    src/file_manager_widget.h
    src/task_manager_widget.cpp
    src/task_manager_widget.h
    src/system_profile.cpp
    src/settings_hub_widget.cpp
    src/settings_hub_widget.h
    src/system_profile.h
)

target_link_libraries(linwin PRIVATE Qt6::Widgets PkgConfig::GIO_UNIX)
target_compile_options(linwin PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
)

include(CTest)
if(BUILD_TESTING)
    add_executable(linwin-task-manager-tests
        tests/test_task_manager.cpp
        src/task_manager_widget.cpp
        src/task_manager_widget.h
    )
    target_include_directories(linwin-task-manager-tests PRIVATE src)
    target_link_libraries(linwin-task-manager-tests PRIVATE Qt6::Widgets)
    add_test(NAME task-manager-tabs COMMAND linwin-task-manager-tests)

    add_executable(linwin-clipboard-tests
        tests/test_clipboard_center.cpp
        src/clipboard_history_widget.cpp
        src/clipboard_history_widget.h
    )
    target_include_directories(linwin-clipboard-tests PRIVATE src)
    target_link_libraries(linwin-clipboard-tests PRIVATE Qt6::Widgets)
    add_test(NAME clipboard-center COMMAND linwin-clipboard-tests)

    add_executable(linwin-settings-hub-tests
        tests/test_settings_hub.cpp
        src/settings_hub_widget.cpp
        src/settings_hub_widget.h
    )
    target_include_directories(linwin-settings-hub-tests PRIVATE src)
    target_link_libraries(linwin-settings-hub-tests PRIVATE Qt6::Widgets)
    add_test(NAME settings-hub-search COMMAND linwin-settings-hub-tests)

    set_tests_properties(task-manager-tabs clipboard-center settings-hub-search PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 20
    )
endif()

include(GNUInstallDirs)
install(TARGETS linwin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/linwin.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/linwin.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES LICENSE README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
set(CPACK_PACKAGE_NAME "linwin")
set(CPACK_PACKAGE_VENDOR "LinWin contributors")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Linux desktop shell engineering preview for Windows switchers")
set(CPACK_PACKAGE_CONTACT "See the project repository for maintainer contacts")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "LinWin contributors")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_GENERATOR "TGZ;DEB;RPM")
include(CPack)
