cmake_minimum_required(VERSION 3.14)

#set(CMAKE_CXX_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/cl.exe")
#set(CMAKE_C_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/cl.exe")
#set(DCMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(diplomski LANGUAGES CXX C)
#project(messageQueueExample LANGUAGES CXX C) 

# Set the C++ standard to C++11
#-DCMAKE_CXX_COMPILER="C:/msys64/mingw64/bin/g++.exe" 

#-DCMAKE_C_COMPILER="C:/msys64/mingw64/bin/gcc.exe"
#set(CMAKE_CXX_STANDARD 11) 
#set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS "-O0 -static -D_FILE_OFFSET_BITS=64")


# Find necessary packages (replace these with your actual package names)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")

#find_package(Vcpkg)
find_package(glfw3 CONFIG REQUIRED)
find_package (imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
#find_package(opengl_system CONFIG REQUIRED)
#set(OpenCV_DIR "${VCPKG_INSTALLED_DIR}/x64-windows/share/opencv4")
find_package(OpenCV REQUIRED) 
# find_package(glad CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
#find_package(GStreamer REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS thread filesystem system)

# Print the names of all Boost libraries found

#find_package(GSTREAMER CONFIG REQUIRED)

#find_package(PkgConfig REQUIRED)

#message(STATUS "GStreamer include directories: ${GSTREAMER_INCLUDE_DIRS}")
#message(STATUS "GStreamer include directories: ${GSTREAMER_LIBRARIES}")
#include_directories(${GSTREAMER_INCLUDE_DIRS})
#find_package(Boost CONFIG REQUIRED)


# Include directories, opencv dir
include_directories(bindings)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glad/include")
#set(OpenCV_DIR C:/opencv-4.9.0/build)
#message(STATUS "***************OpenCV_DIR: ${OpenCV_DIR}")

#include_directories(${OpenCV_INCLUDE_DIRS})
#include_directories(${GSTREAMER_INCLUDE_DIRS})
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
#include_directories(${GSTREAMER_INCLUDE_DIRS})
#link_directories(${GSTREAMER_LIBRARY_DIRS})

include_directories("./3rdParty/imnodes")
include_directories("./include")
#include_directories(${GSTREAMER_INCLUDE_DIRS})
# List your source files

# here I put all implementations (.cpp) files of my code
set(SOURCES 
    src/main.cpp
    src/ConnectionBase.cpp
    src/ConnectorBase.cpp
    src/NodeBase.cpp
    src/NodeTest.cpp
    src/NodeTest2.cpp
    src/ConnectorIn.tpp
    src/ConnectorOut.tpp
    src/ElementFactory.cpp
    src/MessageBase.cpp
    src/Message.tpp
    src/NodeSource.cpp
    src/NodeSourceStream.cpp
    src/NodeInflate.cpp
    src/NodeBackgroundSubstraction.cpp
    src/NodeBlobCreator.cpp
    src/GlobalParams.cpp
    src/FrameBase.cpp
    src/SpatialSpace.cpp
    src/FrameCam.cpp
    src/FrameCustom.cpp
    src/FrameRelation.cpp
    src/NodeBlobGrouper.cpp
    src/NodeManualExtrinsic.cpp
    src/NodeDelayMesurmente.cpp
    src/NodeArucoTracking.cpp

    #src/SimpleClass.cpp
    #src/AwGui.cpp
    #src/TemplateExample.tpp
    #src/ConnectorBase.cpp
    #src/ConnectorIn.cpp
    #src/ConnectorOut.cpp
    #src/ElementFactory.cpp
    #src/NodeBase.cpp
    #src/NodeTest.cpp
)

add_executable(diplomski
    ${SOURCES}
    3rdParty/glad/src/glad.c
    bindings/imgui_impl_glfw.cpp
    bindings/imgui_impl_opengl3.cpp
    3rdParty/imnodes/imnodes.cpp
)
#add_executable(diplomski.exe src/messageQueueExample.cpp ${BOOST_LIBS})
#target_link_libraries(diplomski PRIVATE boost::boost)


#target_include_directories(diplomski PRIVATE ${Boost_INCLUDE_DIRS})

#message(STATUS "***************OpenCV_LIBS: ${OpenCV_LIBS}")
# Link libraries
#link_directories(${GSTREAMER_LIBRARY_DIRS})

target_link_libraries(diplomski
    PRIVATE 
    glfw
    #glad::glad
    imgui::imgui
    implot::implot
    ${OpenCV_LIBS}
    #opengl::opengl
    #Boost::thread
    Boost::boost
    #${GSTREAMER_LIBRARIES}
    # glad::glad
    nlohmann_json::nlohmann_json
    #${GSTREAMER_LIBRARIES}
    #${GSTREAMER_LIBRARIES}
)


#set_target_properties(diplomski PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
#)

#message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")