cmake_minimum_required(VERSION 3.21)
project(LinWin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO_UNIX REQUIRED IMPORTED_TARGET gio-unix-2.0)

set(LINWIN_SHELL_SOURCES
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
    src/setup_wizard.cpp
    src/setup_wizard.h
)

qt_add_executable(linwin src/main.cpp ${LINWIN_SHELL_SOURCES})

target_link_libraries(linwin PRIVATE Qt6::Widgets PkgConfig::GIO_UNIX)
target_compile_options(linwin PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
)

include(GNUInstallDirs)
target_compile_definitions(linwin PRIVATE
    LINWIN_SESSION_HELPER_PATH="${CMAKE_INSTALL_FULL_LIBEXECDIR}/linwin-session-helper"
    LINWIN_UPDATE_HELPER_PATH="${CMAKE_INSTALL_FULL_LIBEXECDIR}/linwin-updater"
)

add_executable(linwin-session-helper src/session_helper.cpp)
target_link_libraries(linwin-session-helper PRIVATE Qt6::Core)
target_compile_options(linwin-session-helper PRIVATE
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

    add_executable(linwin-setup-wizard-tests
        tests/test_setup_wizard.cpp
        src/setup_wizard.cpp
        src/setup_wizard.h
        src/system_profile.cpp
        src/system_profile.h
    )
    target_include_directories(linwin-setup-wizard-tests PRIVATE src)
    target_link_libraries(linwin-setup-wizard-tests PRIVATE Qt6::Widgets)
    target_compile_definitions(linwin-setup-wizard-tests PRIVATE LINWIN_SESSION_HELPER_PATH="/nonexistent/linwin-session-helper")
    add_test(NAME setup-wizard-review COMMAND linwin-setup-wizard-tests)

    add_executable(linwin-file-manager-tests
        tests/test_file_manager.cpp
        src/file_manager_widget.cpp
        src/file_manager_widget.h
    )
    target_include_directories(linwin-file-manager-tests PRIVATE src)
    target_link_libraries(linwin-file-manager-tests PRIVATE Qt6::Widgets PkgConfig::GIO_UNIX)
    target_compile_options(linwin-file-manager-tests PRIVATE
        "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
    )
    add_test(NAME file-manager-quick-access COMMAND linwin-file-manager-tests)

    add_executable(linwin-xfce-dependency-plan-tests tests/test_xfce_dependency_plan.cpp)
    target_include_directories(linwin-xfce-dependency-plan-tests PRIVATE src)
    target_link_libraries(linwin-xfce-dependency-plan-tests PRIVATE Qt6::Core)
    add_test(NAME xfce-dependency-plan COMMAND linwin-xfce-dependency-plan-tests)
    add_executable(linwin-taskbar-layout-tests
        tests/test_taskbar_layout.cpp
        ${LINWIN_SHELL_SOURCES}
    )
    target_include_directories(linwin-taskbar-layout-tests PRIVATE src)
    target_link_libraries(linwin-taskbar-layout-tests PRIVATE Qt6::Widgets PkgConfig::GIO_UNIX)
    target_compile_definitions(linwin-taskbar-layout-tests PRIVATE LINWIN_SESSION_HELPER_PATH="/nonexistent/linwin-session-helper")
    target_compile_options(linwin-taskbar-layout-tests PRIVATE
        "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
    )
    add_test(NAME responsive-taskbar COMMAND linwin-taskbar-layout-tests)
    add_test(NAME x11-session-wrapper
        COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_session_wrapper.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/data/linwin-session)

    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME in-app-updater
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_updater.py)

    set_tests_properties(task-manager-tabs clipboard-center settings-hub-search setup-wizard-review file-manager-quick-access xfce-dependency-plan responsive-taskbar x11-session-wrapper in-app-updater PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 20
    )
endif()

install(TARGETS linwin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS linwin-session-helper RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS scripts/linwin_updater.py DESTINATION ${CMAKE_INSTALL_LIBEXECDIR} RENAME linwin-updater)
install(PROGRAMS data/linwin-session DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES data/linwin.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/linwin-setup.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/linwin-uninstall.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/linwin.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES LICENSE README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
set(CPACK_PACKAGE_NAME "linwin")
set(CPACK_PACKAGE_VENDOR "LinWin contributors")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Windows-style Linux desktop overlay preview")
set(CPACK_PACKAGE_CONTACT "See the project repository for maintainer contacts")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "LinWin contributors")
set(CPACK_DEBIAN_PACKAGE_RELEASE "9")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "python3")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_RELEASE "9")
set(CPACK_RPM_PACKAGE_REQUIRES "python3")
set(CPACK_GENERATOR "TGZ;DEB;RPM")
include(CPack)
