cmake_minimum_required(VERSION 3.16)

project(Game VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 创建third_party目录（如果不存在）
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# 下载nlohmann/json单头文件（如果不存在）
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json.hpp")
    message(STATUS "Downloading nlohmann/json...")
    file(DOWNLOAD
        https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json.hpp
        STATUS download_status
    )

    list(GET download_status 0 error_code)
    if(error_code)
        message(WARNING "Failed")
    endif()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
        main.cpp
        Character.cpp
        Data.cpp
        findpath.cpp
        MyApp.cpp
        plot.cpp
        GameWindow.cpp
        store.cpp
        savemanager.cpp
        JsonLoader.cpp
)

# 添加头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Game
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        MyApp.ui
        Character.h
        findpath.h
        MyApp.h
        Data.h
        plot.h
        GameWindow.h
        store.h
        savemanager.h
            JsonLoader.h
        resources.qrc
    )
else()
    if(ANDROID)
        add_library(Game SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Game
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Game PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)

# 确保资源文件被正确复制
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/levels)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/levels)
    file(COPY levels DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Game)
endif()
set_target_properties(Game PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Game
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Game)
endif()
