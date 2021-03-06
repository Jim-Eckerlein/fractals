cmake_minimum_required(VERSION 3.0)
project(vulkan-fractals)

# Add fmt library:
add_subdirectory(fmt)

# Add GLFW 3 library:
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw-3.2.1)

# Compiler and Linker options:

set(CMAKE_CXX_STANDARD 14)

include_directories(inc)
if (WIN32)
    link_directories(lib-win32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
endif ()

# Executable:
set(SOURCE_FILES main.cpp Application.cpp Application.h util.h util.cpp Vertex.cpp Vertex.h Buffer.cpp Buffer.h StagingBuffer.cpp StagingBuffer.h Pipeline.cpp Pipeline.h Swapchain.cpp Swapchain.h QueueFamilyIndices.h SwapchainParams.h)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries:
if (WIN32)
    target_link_libraries(${PROJECT_NAME} vulkan-1 glfw fmt)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} vulkan glfw fmt)
endif ()

# GLSL shaders:

file(GLOB_RECURSE GLSL_SOURCE_FILES shaders/*.frag shaders/*.vert)

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND glslangValidator -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(spirv-shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${PROJECT_NAME} spirv-shaders)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
        )
