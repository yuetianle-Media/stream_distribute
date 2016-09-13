# CMakeLists templeate
#使用说明
#CMake xx(路径名) -DENABLE_DEBUG=ON(Debug) OFF(Release)
#example CMake . -DENABLE_DEBUG=ON

#CMake 版本支持(最低版本)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#工程名
PROJECT(xxx)

#c11支持
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
#add_compile_options(-std=c++11) #more than GCC4.7

#工程版本类型
OPTION(ENABLE_DEBUG "enable debug" ON)
if(ENABLE_DEBUG STREQUAL ON)
    MESSAGE(STATUS "Enable Debug Build")
    SET(CMAKE_BUILD_TYPE Debug)
else(ENABLE_DEBUG STREQUAL ON)
    MESSAGE(STATUS "Enable Release Build")
    SET(CMAKE_BUILD_TYPE Release)
endif(ENABLE_DEBUG STREQUAL ON)

#文件结构
#  |---------------
#  | ---incl:头文件
#  | ---src:源文件
#  | ---lib:库文件
#  | ---CMakeLists.txt  

#boost 库设置
set (BOOST_ROOT "/usr/local/include/boost/")
#set (Boost_USE_STATIC_LIBS ON)
#set (Boost_USE_MULTITHREADED ON)
#set (Boost_USE_STATIC_RUNTIME OFF)

find_package (Boost 1.61.0 REQUIRED COMPONENTS filesystem system regex date_time coroutine)
if (NOT Boost_FOUND)
    message("not found boost")
endif()

#头文件包含目录 note:一个目录一行
INCLUDE_DIRECTORIES(
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/incl
    ${Boost_INCLUDE_DIRS}
    )

#动态库链接目录 note:一个目录一行
LINK_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR/lib}
    )

#文件依赖关系
AUX_SOURCE_DIRECTORY(src DIR_SRC)

#note 动态库和执行程序 二者选择其一
#动态库
#ADD_LIBRARY(${PROJECT_NAME} ${DIR_SRC})

#执行程序
ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRC})

#动态库版本
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION 1.1 SOVERSION 1)

#链接其他动态库
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES})
