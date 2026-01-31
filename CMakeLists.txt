cmake_minimum_required(VERSION 3.20)

project(usbscope LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets DBus Gui Svg)
find_library(UDEV_LIB udev REQUIRED)

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS src/core/*.cpp)
file(GLOB DAEMON_SOURCES CONFIGURE_DEPENDS src/daemon/*.cpp)
file(GLOB GUI_SOURCES CONFIGURE_DEPENDS src/gui/*.cpp)
file(GLOB UI_SOURCES CONFIGURE_DEPENDS src/ui/*.cpp)
file(GLOB TRAY_SOURCES CONFIGURE_DEPENDS src/tray/*.cpp)

add_library(usbscopecore STATIC ${CORE_SOURCES})
target_link_libraries(usbscopecore PUBLIC Qt6::Core Qt6::DBus)
target_include_directories(usbscopecore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
target_include_directories(usbscopecore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/daemon)

add_library(usbscopegui STATIC ${GUI_SOURCES})
target_link_libraries(usbscopegui PUBLIC Qt6::Core Qt6::Gui Qt6::Svg)
target_include_directories(usbscopegui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/gui)

add_executable(usbscoped ${DAEMON_SOURCES})
target_link_libraries(usbscoped PRIVATE usbscopecore Qt6::Core Qt6::DBus ${UDEV_LIB})
target_include_directories(usbscoped PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

add_executable(usbscope-ui ${UI_SOURCES})
target_link_libraries(usbscope-ui PRIVATE usbscopecore usbscopegui Qt6::Core Qt6::DBus Qt6::Widgets)
target_include_directories(usbscope-ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core ${CMAKE_CURRENT_SOURCE_DIR}/src/gui)

add_executable(usbscope-tray ${TRAY_SOURCES})
target_link_libraries(usbscope-tray PRIVATE usbscopecore usbscopegui Qt6::Core Qt6::DBus Qt6::Widgets)
target_include_directories(usbscope-tray PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core ${CMAKE_CURRENT_SOURCE_DIR}/src/gui)

install(TARGETS usbscoped usbscope-ui usbscope-tray
    RUNTIME DESTINATION bin
)
