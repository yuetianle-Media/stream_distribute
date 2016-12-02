CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(stream_distribute)

#设置Git版本号
find_package(Git)
if(GIT_FOUND)
    message("come here")
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
set (BOOST_ROOT "/usr/local/include/boost/")
#set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)
#set (Boost_USE_STATIC_RUNTIME OFF)
find_package (Boost 1.61.0 REQUIRED COMPONENTS filesystem system regex date_time coroutine thread)
if (NOT Boost_FOUND)
    message("not found boost")
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
