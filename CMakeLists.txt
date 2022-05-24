cmake_minimum_required(VERSION 3.5)

project(pbrt-v4-tool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#设置文件编码
# add_compile_options("/utf-8>")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /utf-8")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL)

set(PROJECT_SOURCES
        main.cpp
        MainDialog.cpp
        MainDialog.h
        MainDialog.ui
        obj2pbrt.cpp obj2pbrt.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(pbrt-v4-tool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET pbrt-v4-tool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(pbrt-v4-tool SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(pbrt-v4-tool
            ${PROJECT_SOURCES}
        )
    endif()
endif()

message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

# set(OpenSceneGraph_DIR "C:/Program Files (x86)/OpenSceneGraph")
set(OSG_DIR "C:/Program Files (x86)/OpenSceneGraph")
find_package(OpenSceneGraph REQUIRED
    COMPONENTS osgGA osgDB osgQt osgManipulator osgViewer)

message("OSG: " ${OPENSCENEGRAPH_LIBRARIES} "," ${OPENSCENEGRAPH_INCLUDE_DIRS})


target_include_directories(pbrt-v4-tool PRIVATE
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)


message("OSG_LIBRARY:" ${OSG_LIBRARY} "," ${OSG_LIBRARY_RELEASE})
target_link_libraries(pbrt-v4-tool PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    ${OPENSCENEGRAPH_LIBRARIES}
)

set_target_properties(pbrt-v4-tool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 复制pbrt文件
file(COPY pbrt DESTINATION .)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(pbrt-v4-tool)
endif()
