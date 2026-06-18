cmake_minimum_required(VERSION 3.15)

project(HeadSoccer3D
        VERSION 1.0
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static libraries" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    set(COMPILER_WARNINGS /W4)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(COMPILER_WARNINGS -Wall -Wextra -pedantic)
    add_compile_options(-O3)
endif()

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG        v2.0-rc1
)
FetchContent_MakeAvailable(tinyobjloader)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.8
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(pybind11)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_subdirectory(external/glad)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC glfw glad)

add_library(HeadSoccerCore STATIC
    src/engine/core/application.cpp
    src/engine/core/debug.cpp
    src/engine/core/input.cpp
    src/engine/core/window.cpp
    src/engine/graphics/mesh.cpp
    src/engine/graphics/model.cpp
    src/engine/graphics/shader.cpp
    src/engine/graphics/renderSystem.cpp
    src/engine/physics/contact.cpp
    src/engine/physics/physicsSystem.cpp
    src/engine/world/scene.cpp
    src/engine/world/baseScene.cpp
    src/engine/world/components/collider.cpp
    src/engine/world/components/rigidbody.cpp
    src/engine/world/components/transform.cpp
    src/game/scripts/playerController.cpp
    src/game/scripts/defaultScene.cpp
    src/game/scripts/menuScene.cpp
    src/game/scripts/cameraController.cpp
    src/game/scripts/footballer.cpp
    src/game/scripts/ball.cpp
    src/game/scripts/playerGrounded.cpp
    src/game/scripts/footballerShootTrigger.cpp
    src/game/scripts/enemyController.cpp
    src/game/scripts/gateTrigger.cpp
    src/game/scripts/freeCameraController.cpp
    src/game/scripts/ai/math.cpp
    src/game/scripts/ai/neuralAgent.cpp
    src/game/scripts/pitchGenerator.cpp
    src/game/scripts/shoeController.cpp
    src/game/scripts/powerup.cpp
    src/third_party/stb_implementation.cpp
    src/third_party/tinyobjloader_implementation.cpp
)

target_compile_options(HeadSoccerCore PRIVATE ${COMPILER_WARNINGS})

target_include_directories(HeadSoccerCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
    ${stb_SOURCE_DIR}
)

target_link_libraries(HeadSoccerCore PUBLIC
    glfw
    glm::glm
    glad
    tinyobjloader
    imgui_lib
)

if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(HeadSoccerCore PUBLIC ${OPENGL_LIBRARIES} dl pthread)
endif()

add_executable(HeadSoccerGame src/main.cpp)

target_link_libraries(HeadSoccerGame PRIVATE HeadSoccerCore)

add_custom_command(TARGET HeadSoccerGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:HeadSoccerGame>/assets"
    COMMENT "Copying assets for Game..."
)

find_package(OpenMP REQUIRED COMPONENTS CXX)

add_executable(AITrainer
    src/trainer/mainTrainer.cpp
    src/trainer/trainerScene.cpp
    src/trainer/geneticTrainer.cpp
    src/trainer/matchArena.cpp
)

target_link_libraries(AITrainer PRIVATE HeadSoccerCore OpenMP::OpenMP_CXX)

add_custom_command(TARGET AITrainer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:AITrainer>/assets"
    COMMENT "Copying assets for AI Trainer..."
)

add_library(HeadSoccerCoreHeadless STATIC
    src/engine/physics/contact.cpp
    src/engine/physics/physicsSystem.cpp
    src/engine/world/scene.cpp
    src/engine/world/baseScene.cpp
    src/engine/world/components/collider.cpp
    src/engine/world/components/rigidbody.cpp
    src/engine/world/components/transform.cpp
    src/game/scripts/footballer.cpp
    src/game/scripts/ball.cpp
    src/game/scripts/playerGrounded.cpp
    src/game/scripts/footballerShootTrigger.cpp
    src/game/scripts/enemyController.cpp
    src/game/scripts/gateTrigger.cpp
    src/game/scripts/powerup.cpp
    src/game/scripts/ai/math.cpp
    src/game/scripts/ai/neuralAgent.cpp
    src/game/scripts/pitchGenerator.cpp
    src/trainer/matchArena.cpp
    src/trainer/headlessApplication.cpp
    src/trainer/headlessTrainerScene.cpp
)

target_compile_definitions(HeadSoccerCoreHeadless PUBLIC HEADLESS_MODE)

target_include_directories(HeadSoccerCoreHeadless PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
    ${stb_SOURCE_DIR}
)

target_link_libraries(HeadSoccerCoreHeadless PUBLIC
    glm::glm
)

if(UNIX AND NOT APPLE)
    target_link_libraries(HeadSoccerCoreHeadless PUBLIC ${OPENGL_LIBRARIES} dl pthread)
endif()

add_executable(AITrainerHeadless
    src/trainer/mainHeadless.cpp
    src/trainer/geneticTrainer.cpp
)

target_compile_definitions(AITrainerHeadless PRIVATE HEADLESS_MODE)

target_link_libraries(AITrainerHeadless PRIVATE
    HeadSoccerCoreHeadless
    OpenMP::OpenMP_CXX
)

if(MINGW)
    target_link_options(AITrainerHeadless PRIVATE "-fuse-ld=lld")
endif()


pybind11_add_module(soccer_engine
    src/trainer/pythonBindings.cpp
    src/trainer/headlessTrainerScene.cpp
)

target_compile_definitions(soccer_engine PRIVATE HEADLESS_MODE)

target_link_libraries(soccer_engine PRIVATE
    HeadSoccerCoreHeadless
    #OpenMP::OpenMP_CXX
)

target_link_options(soccer_engine PRIVATE "-fuse-ld=lld")
