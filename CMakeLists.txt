cmake_minimum_required(VERSION 3.16)

project(meeting VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(appmeeting
    main.cpp
)

qt_add_qml_module(appmeeting
    URI meeting
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES mysql.cpp
        SOURCES mysqlManager.h
        SOURCES mysqlManager.cpp
        SOURCES socket.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appmeeting PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appmeeting
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appmeeting
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appmeeting
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
