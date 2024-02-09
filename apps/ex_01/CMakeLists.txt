add_executable(
        absorb_demo
        src/main.cc
        ${IMGUI_SOURCES}
)

target_include_directories(absorb_demo
        PRIVATE
        ${imgui_SOURCE_DIR}
)

target_link_libraries(absorb_demo
        PRIVATE
        GLHelpers
        ${OpenGL_LIBRARY}
        glfw)

# Copy shaders
add_custom_target(absorb_demo_shaders)
add_custom_command(
        TARGET absorb_demo_shaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ex01.frag ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ex01.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
        COMMENT "Copying shaders..."
)
add_dependencies (absorb_demo absorb_demo_shaders)
