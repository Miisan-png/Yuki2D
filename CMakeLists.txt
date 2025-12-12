cmake_minimum_required(VERSION 3.16)
project(Yuki2D)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
add_executable(yuki2d
    src/main.cpp
    src/core/window.cpp
    src/core/renderer2d.cpp
    src/core/log.cpp
    src/core/time.cpp
    src/core/input.cpp
    src/core/engine_bindings.cpp
    src/core/bindings/state.cpp
    src/core/bindings/binding_core.cpp
    src/core/bindings/binding_render.cpp
    src/core/bindings/binding_anim.cpp
    src/core/bindings/binding_collision.cpp
    src/core/bindings/binding_input.cpp
    src/core/bindings/binding_tween.cpp
    src/core/bindings/binding_math.cpp
    src/core/bindings/core_api.cpp
    src/core/bindings/collision_api.cpp
    src/core/bindings/tween_api.cpp
    src/core/bindings/input_api.cpp
    src/core/bindings/anim_api.cpp
    src/core/bindings/render_api.cpp
    src/core/bindings/math_api.cpp
    src/script/yuki_script_loader.cpp
    src/script/tokenizer.cpp
    src/script/token_debug.cpp
    src/script/parser.cpp
    src/script/ast_debug.cpp
    src/script/value.cpp
    src/script/builtins.cpp
    src/script/interpreter.cpp
    src/script/environment.cpp
    src/runtime/yuki_runner.cpp
    src/runtime/dev_console.cpp)
target_include_directories(yuki2d PRIVATE src/core src/script src/runtime src)
target_link_libraries(yuki2d PRIVATE glfw OpenGL::GL)
target_compile_options(yuki2d PRIVATE -Wall -Wextra -Wpedantic)
