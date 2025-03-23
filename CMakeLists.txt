cmake_minimum_required(VERSION 3.16)

project(MqttPlugin VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

if(NOT CMAKE_CROSSCOMPILING AND NOT DISABLE_PKG)
    if(EXISTS /etc/debian_version)
        message(STATUS "Build for debian package")
        set(BUILD_DEBIAN_PACKAGE ON)
    endif()
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)
find_package(eclipse-paho-mqtt-c)
find_package(KaZa REQUIRED)
include_directories(${KAZA_INCLUDE_DIR})

add_library(
    MqttPlugin
    SHARED
    src/plugin.h        src/plugin.cpp
    src/mqttclient.h    src/mqttclient.cpp
    src/mqtttopic.h     src/mqtttopic.cpp
    qmldir
)

if(NOT DEFINED QML_MODULE_INSTALL_PATH)
    if(EXISTS /usr/lib64/qt6/qml/builtins.qmltypes)
        set(QML_MODULE_INSTALL_PATH "/usr/lib64/qt6/qml/")
    elseif(EXISTS /usr/lib/aarch64-linux-gnu/qt6/qml/builtins.qmltypes)
        set(QML_MODULE_INSTALL_PATH "/usr/lib/aarch64-linux-gnu/qt6/qml/")
    else()
        set(QML_MODULE_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/qml )
    endif()
endif()

target_compile_definitions(MqttPlugin PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(MqttPlugin PRIVATE Qt6::Quick eclipse-paho-mqtt-c::paho-mqtt3a)
target_include_directories(MqttPlugin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS MqttPlugin DESTINATION ${QML_MODULE_INSTALL_PATH}/org/kazoe/mqtt)
install(FILES qmldir DESTINATION ${QML_MODULE_INSTALL_PATH}/org/kazoe/mqtt)


if(BUILD_DEBIAN_PACKAGE)
    set(DEB_DEPEND "")
    file(READ /etc/debian_version DEBIAN_VERSION)
    string(REGEX MATCH "^[0-9]+" DEBIAN_MAJOR "${DEBIAN_VERSION}")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "kaza-mqtt")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_PACKAGE_VERSION_PATCH "${CMAKE_PROJECT_VERSION_PATCH}-debian${DEBIAN_MAJOR}")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpaho-mqtt1.3, kaza-server-bin, libqt6core6")
    set(CPACK_PACKAGE_DESCRIPTION "MQTT integration for KaZa Server")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Fabien Proriol <fabien.proriol@kazoe.org>")
    include(CPack)
endif()
