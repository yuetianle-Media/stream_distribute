CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(stream_distribute)

#设置Git版本号
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=40 OUTPUT_VARIABLE DESCRIBE_BUILD OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
MESSAGE("GIT VERSION:" ${DESCRIBE_BUILD})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/incl/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/incl/version.h)

set(CMAKE_CXX_COMPILER g++)
add_compile_options(-std=c++11)
OPTION(ENABLE_DEBUG "enable debug" ON)
if(ENABLE_DEBUG STREQUAL ON)
    MESSAGE(STATUS "Enable Debug Build")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Os -pthread")
    SET(CMAKE_BUILD_TYPE Debug)
else(ENABLE_DEBUG STREQUAL ON)
    MESSAGE(STATUS "Enable Release Build")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread ")
    SET(CMAKE_BUILD_TYPE Release)
endif(ENABLE_DEBUG STREQUAL ON)
#boost 设定
set (BOOST_ROOT "/usr/local/include/boost/")
OPTION(USE_STATIC_BOOST "build with static boost instead of dynamic" ON)
set(Boost_USE_STATIC_LIBS ${USE_STATIC_BOOST})
set (Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
unset(Boost_INCLUDE_DIR CACHE)
unset(Boost_LIBRARY_DIRS CACHE)
#find_package(Boost REQUIRED COMPONENTS thread program_options system)
find_package (Boost 1.61.0 REQUIRED COMPONENTS filesystem system regex date_time coroutine thread program_options)
if(USE_STATIC_BOOST)
   message(STATUS "Linking against boost static libraries")
else(USE_STATIC_BOOST)
   message(STATUS "Linking against boost dynamic libraries")
endif(USE_STATIC_BOOST)
#set(Boost_USE_STATIC_LIBS ${USE_STATIC_BOOST})
#unset(Boost_INCLUDE_DIR CACHE)
#unset(Boost_LIBRARY_DIRS CACHE)
#set (Boost_USE_MULTITHREADED ON)
#set (Boost_USE_STATIC_RUNTIME ON)
#if(USE_STATIC_BOOST)
#    MESSAGE(STATUS "linking boost with static libs.")
#else(USE_STATIC_BOOST)
#    MESSAGE(STATUS "linking boost with dynamic libs.")
#endif(USE_STATIC_BOOST)
#find_package (Boost 1.61.0 REQUIRED COMPONENTS filesystem system regex date_time coroutine thread program_options)
if (NOT Boost_FOUND)
    message("not found boost")
else()
    message("boost found lib:" ${Boost_LIBRARIES})
endif()
SET(requiredlibs)
FIND_PACKAGE(CURL)
if(CURL_FOUND)
    message("curl foud")
    INCLUDE_DIRECTORIES($(CURL_INCLUDE_DIR))
    SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES})
ELSE(CURL_FOUND)
    message(FATAL_ERROR "could not find curl library")
ENDIF(CURL_FOUND)

INCLUDE_DIRECTORIES(
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/incl
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/extlib/votlogger/include
    )

LINK_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR/extlib/votlogger/lib}
    )

AUX_SOURCE_DIRECTORY(src DIR_SRC)

ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRC})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES} ${requiredlibs} vistek_logger)
#包含cmake特定模块目录
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#程序输出目录
set(EXECUTEABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
#cpack setting
include(CPackConfig)
#第三方库安装
#install program
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/ LIBRARY DESTINATION lib/)
