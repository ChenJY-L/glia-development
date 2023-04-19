cmake_minimum_required(VERSION 3.21)
project(GLIA_ART)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# 自动调用moc，uic，rcc处理qt的扩展部分
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
if (WIN32)
    # 设置外部库路径
    set(CMAKE_PREFIX_PATH "E:/Program/Qt/5.13.0/msvc2017_64/lib/cmake")
    set(MVS_INCLUDE "E:/Program/MVS/Development/Includes")
    set(MVS_LIBS "E:/Program/MVS/Development/Libraries/win64")
    set(Qt5_DIR "E:/Program/Qt/5.13.0/msvc2017_64")
    set(OpenCV_DIR "E:/Program/opencv/build/x64/vc15/lib")
    set(ART_INCLUDE "C:/Program Files (x86)/ART Technology/ArtDAQ/Cards/USB3106A/Include")
    set(ART_LIB "C:/Program Files (x86)/ART Technology/ArtDAQ/Cards/USB3106A/Lib/x64")
elseif(UNIX)
    set(MVS_INCLUDE /opt/MVS/include)
    set(MVS_LIBS /opt/MVS/lib/64)
endif()
# 设置采集卡驱动路径
# 调用MVS库函数
# 调用ROOT绘图库
find_package(ROOT 6.26 CONFIG REQUIRED)
set(CMAKE_CXX_FLAGS "${ROOT_CXX_FLAGS}")
# 调用Qt库函数
find_package(Qt5 
        COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV Include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV Libs: ${OpenCV_LIBS}")
message(STATUS "Qt5 Include: ${Qt5Widgets_INCLUDE_DIRS}")
message(STATUS "Qt5 Libs: ${Qt5Widgets_LIBRARIES}")
message(STATUS "MVS Include: ${MVS_INCLUDE}")
message(STATUS "MVS Libs: ${MVS_LIBS}")
message(STATUS "ROOT Include: ${ROOT_INCLUDE_DIRS}")
message(STATUS "ROOT Libs: ${ROOT_DEFINITIONS}")

include_directories(${MVS_INCLUDE})
link_directories(${MVS_LIBS})
#include_directories(${ART_DIR})
include_directories(${ART_INCLUDE})
link_directories(${ART_LIB})    # 导入采集卡头文件路径
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp mainwindow.cpp mainwindow.h mainwindow.ui
        ART/artwindow.cpp ART/artwindow.h ART/artwindow.ui
        cam/CamWindow.cpp cam/CamWindow.h cam/camwindow.ui
        MvCamera/MvCamera.cpp MvCamera/MvCamera.h
        canvas/canvas.cpp canvas/canvas.h
        algorithm/algorithm.cpp algorithm/algorithm.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
endif()

target_link_libraries(${PROJECT_NAME} ${ROOT_LIBRARIES} )
target_link_libraries(${PROJECT_NAME}  ${ART_LIB}/USB3106A_64.lib)
target_link_libraries(${PROJECT_NAME}  ${MVS_LIBS}/MvCameraControl.lib)
target_link_libraries(${PROJECT_NAME}  Qt5::Widgets)
target_link_libraries(${PROJECT_NAME}  Qt5::Gui)
target_link_libraries(${PROJECT_NAME}  Qt5::Core)
target_link_libraries(${PROJECT_NAME}  ${OpenCV_LIBS})
# target_link_libraries(${PROJECT_NAME}  stdc++fs)
