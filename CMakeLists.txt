cmake_minimum_required(VERSION 3.16)

project(sprite-opera VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)

set(CMAKE_CURRENT_SOURCE_DIR /home/giantbooley/code/sprite-opera/)

set(PROJECT_SOURCES
    main.cpp
    include/ImageData.hpp ImageData.tpp
    include/IImageData.hpp
    include/sprite.hpp sprite.cpp
    include/glwidget.h glwidget.cpp
    mainwindow.cpp
    include/mainwindow.h
    mainwindow.ui
    include/math.hpp math.cpp
    include/stringcreator.hpp stringcreator.cpp
    include/binarymask.hpp binarymask.cpp
    include/normalmapgenerator.hpp normalmapgenerator.cpp
    include/spriteopera.h spriteopera.cpp
)
include_directories(include)

qt_add_executable(sprite-opera MANUAL_FINALIZATION ${PROJECT_SOURCES})

# Link all required Qt6 modules
target_link_libraries(sprite-opera PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
)
target_link_libraries(sprite-opera PRIVATE Qt6::Widgets)
target_link_libraries(sprite-opera PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Bundle properties
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.sprite-opera)
endif()
set_target_properties(sprite-opera PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS sprite-opera
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(sprite-opera)
endif()
