cmake_minimum_required(VERSION 3.21)
project(LinWin VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

qt_add_executable(linwin
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
    src/clipboard_history_widget.cpp
    src/clipboard_history_widget.h
    src/system_profile.cpp
    src/system_profile.h
)

target_link_libraries(linwin PRIVATE Qt6::Widgets)
target_compile_options(linwin PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>"
)

install(TARGETS linwin RUNTIME DESTINATION bin)
install(FILES data/linwin.desktop DESTINATION share/applications)
install(FILES data/linwin.svg DESTINATION share/icons/hicolor/scalable/apps)

include(GNUInstallDirs)
set(CPACK_PACKAGE_NAME "linwin")
set(CPACK_PACKAGE_VENDOR "LinWin contributors")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-distro Linux desktop usability tools")
set(CPACK_PACKAGE_CONTACT "See the project repository for maintainer contacts")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "LinWin contributors")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_GENERATOR "TGZ;DEB;RPM")
include(CPack)
