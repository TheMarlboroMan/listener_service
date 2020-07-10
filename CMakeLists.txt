cmake_minimum_required(VERSION 3.5.1)
project(sck)

option(BUILD_EXAMPLES "Build example code" ON)
option(BUILD_DEBUG "Build a debug release" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(${BUILD_DEBUG})
        set(CMAKE_BUILD_TYPE Debug)

else()
        set(CMAKE_BUILD_TYPE Release)
endif()


add_compile_options(-Wall -Wextra -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-deprecated -ansi -pedantic -fno-rtti)

include_directories("${PROJECT_SOURCE_DIR}/include")
set(SOURCE "")

add_subdirectory("${PROJECT_SOURCE_DIR}/lib")
add_library(sck_static STATIC ${SOURCE})
set_target_properties(sck_static PROPERTIES OUTPUT_NAME sck)

add_library(sck_shared SHARED ${SOURCE})
set_target_properties(sck_shared PROPERTIES OUTPUT_NAME sck)

install(TARGETS sck_shared DESTINATION lib)
install(TARGETS sck_static DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

if(${BUILD_EXAMPLES})

	#External libs.
	add_library(lm STATIC IMPORTED)
	add_library(tools STATIC IMPORTED)

	set_target_properties(tools PROPERTIES IMPORTED_LOCATION /usr/local/lib/libtools.a)
	set_target_properties(lm PROPERTIES IMPORTED_LOCATION /usr/local/lib/liblm.a)

	add_executable(client example/client.cpp example/src/client.cpp )
	target_link_libraries(client sck_shared tools lm pthread)

	add_executable(server example/server.cpp example/src/example_logic.cpp )
	target_link_libraries(server sck_shared tools lm pthread)

endif()
