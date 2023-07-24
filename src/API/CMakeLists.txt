#1. 确定CMake的最低版本需求
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)

#2. 确定工程名
PROJECT(ccxtcpp LANGUAGES CXX)
SET(CMAKE_CXX_STANDARD 17)

IF(MSVC)
ELSE(GNUCC)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/libs)

# 查找 src 目录下的所有源文件
file(GLOB INC_FILES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/websocket_client/*.h
)

file(GLOB LIB_INC_FILES
    ${PROJECT_SOURCE_DIR}/*.h
    ./websocket_client/ws_callback.h
    ./websocket_client/ws_client.h
)

file(GLOB SRC_FILES
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/websocket_client/*.cpp
)

INCLUDE_DIRECTORIES(${INCS})
LINK_DIRECTORIES(${LNKS})

# 生成静态库
add_library(${PROJECT_NAME} STATIC ${INC_FILES} ${SRC_FILES})

# 生成动态库
# add_library(${PROJECT_NAME} SHARED ${INC_FILES} ${SRC_FILES})

set (LIBS curl jsoncpp websockets cpr)

# target_link_libraries(${PROJECT_NAME} ${LIBS})
target_link_libraries(${PROJECT_NAME} ${LIBS})
# set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
