cmake_minimum_required(VERSION 3.22)

project(mio2it
	VERSION 0.1.0
	LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Trim down binary size, and show more compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-ffunction-sections -Wall -Wextra)
	add_link_options(-Wl,--gc-sections)

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		add_compile_options(-Xclang -fmerge-functions -fnew-infallible)
		add_link_options(-fuse-ld=lld -Wl,--icf=safe)
	endif()
endif()

add_executable(mio2it
	src/io.cpp
	src/it.cpp
	src/main.cpp
	src/mio.cpp
)

configure_file(src/version.hpp.in src/version.hpp)

target_compile_definitions(mio2it PRIVATE "$<$<CONFIG:DEBUG>:MIO2IT_DEBUG>")
target_compile_features(mio2it PUBLIC cxx_std_23)
target_include_directories(mio2it PRIVATE
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_BINARY_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS mio2it)
