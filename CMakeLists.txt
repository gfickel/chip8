project(chip8)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -std=c++11 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -std=c++11 -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

include_directories("third_party/"
					"third_party/imgui/"
					"third_party/imgui/examples/"
                    "third_party/gl3w/"
                    "third_party/TinySoundFont/"
                    )

file(GLOB all_chip8_src
    "src/*.cpp"
	"third_party/imgui/examples/imgui_impl_glfw.cpp"
	"third_party/imgui/examples/imgui_impl_opengl3.cpp"
	"third_party/imgui/imgui.cpp"
	"third_party/imgui/imgui_draw.cpp"
	"third_party/imgui/imgui_widgets.cpp"
    "third_party/gl3w/GL/gl3w.c"
    "third_party/TinySoundFont/minisdl_audio.c"
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(OpenGL REQUIRED)


add_executable(chip8 ${all_chip8_src})

if(APPLE)
    set(FRAMEWORK_COCOA "-framework Cocoa" CACHE STRING "Cocoa framework for OSX")
    set(FRAMEWORK_COREVIDEO "-framework CoreVideo" CACHE STRING "CoreVideo framework for OSX")
    set(FRAMEWORK_IOKIT "-framework IOKit" CACHE STRING "IOKit framework for OSX")

    target_link_libraries(chip8 ${OPENGL_LIBRARIES} ${FRAMEWORK_COCOA} ${FRAMEWORK_COREVIDEO} ${FRAMEWORK_IOKIT} ${GLFW_STATIC_LIBRARIES})
elseif(UNIX)
    target_link_libraries(chip8 pthread GL ${GLFW_STATIC_LIBRARIES})
endif()
