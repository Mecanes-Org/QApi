cmake_minimum_required(VERSION 3.16)
project(testApi LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Test)

# ── PAS de add_subdirectory(../QApi) ici ──
# QApi est déjà déclaré par le CMakeLists.txt parent

add_executable(testApi tst_testapi.cpp)

add_test(NAME testApi COMMAND testApi)

target_link_libraries(testApi
    PRIVATE
        QApi                               # hérité du parent
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Test
)