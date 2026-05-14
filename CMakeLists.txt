cmake_minimum_required(VERSION 3.16)

project(QApi LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_library(QApi SHARED
  QApi_global.h
  qapi.cpp
  qapi.h
)

add_library(QApi::QApi ALIAS QApi)

# ── PUBLIC pour que les projets hérite du path automatiquement ──
target_include_directories(QApi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(QApi
    PUBLIC
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Network
)


# target_link_libraries(QApi PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(QApi PRIVATE QAPI_LIBRARY)
# Suprimer
enable_testing()
add_subdirectory(QApiTest)