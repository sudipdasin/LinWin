cmake_minimum_required(VERSION 3.21)
project(LinWin VERSION 1.0.1 LANGUAGES CXX)
set(LINWIN_BUILD_VERSION "${PROJECT_VERSION}" CACHE STRING "Version displayed by the executable")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)
add_compile_definitions(LINWIN_UPDATE_WORKER_PATH="${CMAKE_INSTALL_FULL_LIBEXECDIR}/linwin/embedded_worker.py")

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Network)
find_package(Qt6 6.4 QUIET COMPONENTS OpenGL)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO_UNIX REQUIRED IMPORTED_TARGET gio-unix-2.0)

set(LINWIN_SHELL_SOURCES
    src/main_window.cpp
    src/main_window.h
    src/desktop_surface.h
    src/wallpaper_shader.h
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
    src/embedded_update_dialog.h
    src/system_profile.h
    src/setup_wizard.cpp
    src/setup_wizard.h
)

qt_add_executable(linwin src/main.cpp ${LINWIN_SHELL_SOURCES})

target_link_libraries(linwin PRIVATE Qt6::Widgets Qt6::Network PkgConfig::GIO_UNIX)
if(TARGET Qt6::OpenGL)
    target_link_libraries(linwin PRIVATE Qt6::OpenGL)
    target_compile_definitions(linwin PRIVATE LINWIN_HAS_OPENGL=1)
endif()
target_compile_options(linwin PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
)

include(GNUInstallDirs)
target_compile_definitions(linwin PRIVATE
    LINWIN_BUILD_VERSION="${LINWIN_BUILD_VERSION}"
    LINWIN_SESSION_HELPER_PATH="${CMAKE_INSTALL_FULL_LIBEXECDIR}/linwin-session-helper"
)

add_executable(linwin-session-helper src/session_helper.cpp)
target_link_libraries(linwin-session-helper PRIVATE Qt6::Core)
target_compile_options(linwin-session-helper PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
)

include(CTest)
if(BUILD_TESTING)
    add_executable(linwin-theme-catalog-tests tests/test_theme_catalog.cpp)
    target_include_directories(linwin-theme-catalog-tests PRIVATE src)
    add_test(NAME theme-catalog COMMAND linwin-theme-catalog-tests)
    add_executable(linwin-snap-geometry-tests tests/test_snap_geometry.cpp)
    target_include_directories(linwin-snap-geometry-tests PRIVATE src)
    add_test(NAME snap-geometry COMMAND linwin-snap-geometry-tests)
    add_executable(linwin-workspace-profile-tests tests/test_workspace_profiles.cpp)
    target_include_directories(linwin-workspace-profile-tests PRIVATE src)
    target_link_libraries(linwin-workspace-profile-tests PRIVATE Qt6::Core)
    add_test(NAME workspace-profiles COMMAND linwin-workspace-profile-tests)
    add_executable(linwin-recent-document-tests tests/test_recent_documents.cpp)
    target_include_directories(linwin-recent-document-tests PRIVATE src)
    target_link_libraries(linwin-recent-document-tests PRIVATE Qt6::Core)
    add_test(NAME recent-documents COMMAND linwin-recent-document-tests)
    add_executable(linwin-neon-landscape-tests tests/test_neon_landscape.cpp)
    target_include_directories(linwin-neon-landscape-tests PRIVATE src)
    target_link_libraries(linwin-neon-landscape-tests PRIVATE Qt6::Gui)
    add_test(NAME neon-landscape COMMAND linwin-neon-landscape-tests)
    add_executable(linwin-terrain-texture-tests tests/test_terrain_texture.cpp)
    target_include_directories(linwin-terrain-texture-tests PRIVATE src)
    add_test(NAME terrain-texture COMMAND linwin-terrain-texture-tests)
    add_executable(linwin-pin-suggestion-tests tests/test_pin_suggestions.cpp)
    target_include_directories(linwin-pin-suggestion-tests PRIVATE src)
    add_test(NAME pin-suggestions COMMAND linwin-pin-suggestion-tests)
    add_executable(linwin-weather-tests tests/test_weather_forecast.cpp)
    target_include_directories(linwin-weather-tests PRIVATE src)
    target_link_libraries(linwin-weather-tests PRIVATE Qt6::Core)
    add_test(NAME weather-forecast COMMAND linwin-weather-tests)
    add_executable(linwin-desktop-status-tests tests/test_desktop_status.cpp)
    target_include_directories(linwin-desktop-status-tests PRIVATE src)
    target_link_libraries(linwin-desktop-status-tests PRIVATE Qt6::Core)
    add_test(NAME desktop-status COMMAND linwin-desktop-status-tests)
    add_executable(linwin-system-metrics-tests tests/test_system_metrics.cpp)
    target_include_directories(linwin-system-metrics-tests PRIVATE src)
    add_test(NAME system-metrics COMMAND linwin-system-metrics-tests)
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
    target_link_libraries(linwin-taskbar-layout-tests PRIVATE Qt6::Widgets Qt6::Network PkgConfig::GIO_UNIX)
    if(TARGET Qt6::OpenGL)
        target_link_libraries(linwin-taskbar-layout-tests PRIVATE Qt6::OpenGL)
        target_compile_definitions(linwin-taskbar-layout-tests PRIVATE LINWIN_HAS_OPENGL=1)
    endif()
    target_compile_definitions(linwin-taskbar-layout-tests PRIVATE LINWIN_SESSION_HELPER_PATH="/nonexistent/linwin-session-helper")
    target_compile_options(linwin-taskbar-layout-tests PRIVATE
        "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
    )
    add_test(NAME responsive-taskbar COMMAND linwin-taskbar-layout-tests)
    add_test(NAME x11-session-wrapper
        COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_session_wrapper.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/data/linwin-session)

    set_tests_properties(task-manager-tabs clipboard-center settings-hub-search setup-wizard-review file-manager-quick-access xfce-dependency-plan responsive-taskbar x11-session-wrapper PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 20
    )
endif()

install(TARGETS linwin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS linwin-session-helper RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS data/linwin-session DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES updater/embedded_worker.py updater/linwin-update updater/release_policy.py updater/update_guard.py
    DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/linwin)
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
set(CPACK_DEBIAN_PACKAGE_RELEASE "8")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "python3 (>= 3.11), apt, dpkg, pkexec, ca-certificates, gpgv")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_RELEASE "8")
set(CPACK_GENERATOR "TGZ;DEB;RPM")
include(CPack)
