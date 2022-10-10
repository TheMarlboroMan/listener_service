cmake_minimum_required(VERSION 3.5.1)
project(sck)

option(BUILD_EXAMPLES "Build example code, only with shared libraries" ON)
option(BUILD_SHARED "Build a shared library" ON)
option(BUILD_STATIC "Build a static library" OFF)
option(BUILD_DEBUG "Build a debug release" OFF)
option(WITH_SSL "Build with SSL" OFF)
option(WITH_SSL_CURRENT "Build current SSL version" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(${BUILD_DEBUG})
	set(CMAKE_BUILD_TYPE Debug)

else()

	set(CMAKE_BUILD_TYPE Release)
endif()

if(${WITH_SSL})

	add_definitions("-DWITH_SSL")
endif()

if(${WITH_SSL_CURRENT})

	add_definitions("-DWITH_CURRENT_SSL")
endif()

add_compile_options(-Wall -Wextra -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-deprecated -ansi -pedantic -fno-rtti)

include_directories("${PROJECT_SOURCE_DIR}/include")
set(SOURCE "")
add_subdirectory("${PROJECT_SOURCE_DIR}/lib")
install(DIRECTORY include/ DESTINATION include)


if(${BUILD_STATIC})

	add_library(sck_static STATIC ${SOURCE})
	set_target_properties(sck_static PROPERTIES OUTPUT_NAME sck)
	install(TARGETS sck_static DESTINATION lib)

endif()

if(${BUILD_SHARED})

	add_library(sck_shared SHARED ${SOURCE})
	set_target_properties(sck_shared PROPERTIES OUTPUT_NAME sck)
	install(TARGETS sck_shared DESTINATION lib)

	if(${BUILD_EXAMPLES})

		#External libs.
		add_library(lm SHARED IMPORTED)
		add_library(tools SHARED IMPORTED)

		set_target_properties(tools PROPERTIES IMPORTED_LOCATION /usr/local/lib/libtools.so)
		set_target_properties(lm PROPERTIES IMPORTED_LOCATION /usr/local/lib/liblm.so)

		add_executable(client example/client.cpp example/src/client.cpp )
		target_link_libraries(client sck_shared tools lm pthread)

		add_executable(server example/server.cpp example/src/example_logic.cpp )
		target_link_libraries(server sck_shared tools lm pthread)

	endif()
ENDIF()
