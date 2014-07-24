cmake_minimum_required(VERSION 2.6)
project(CryptoTest)
set(CryptoText_VERSION_MAJOR 1)
set(CryptoText_VERSION_MINOR 0)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -ggdb -O0 -pg -Wall -Wextra -Wno-unused --std=c++11")
ADD_CUSTOM_TARGET(Profile
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Profile ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  COMMENT "Switch CMAKE_BUILD_TYPE to Profile"
  )
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS} -g3 -ggdb -O0 -pg -Wall -Wextra -Wno-unused --std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g3 -ggdb -O0 -Wall -Wextra -Wno-unused --std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused --std=c++11")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-DDEBUG)
endif ()
find_library(cryptopp SHARED /usr/lib/libcryptopp.so)
find_library(pthread SHARED /usr/lib/libpthread.so.0)
add_executable(TestProgram.exe TestProgram.cpp)
target_link_libraries(TestProgram.exe cryptopp pthread)
