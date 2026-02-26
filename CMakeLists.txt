cmake_minimum_required(VERSION 3.10)

project(BudgetApp LANGUAGES CXX C)

set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)

set(CMAKE_CXX_STANDARD 20)

include_directories(
    external/imgui
    external/imgui/backends
    external/sqlite
)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCES
    src/future_main.cpp
    src/app_controller.cpp

    src/UI/sidebar_panel.cpp
    src/UI/right_panel.cpp
    src/UI/create_account_panel.cpp
    src/UI/modify_account_panel.cpp
    src/UI/account_view_panel.cpp
    src/UI/transaction_form.cpp
    src/UI/latest_transactions_table.cpp

    src/core_logic.cpp
    src/storage.cpp
    src/helpers.cpp

    # ImGui (C++)
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/misc/cpp/imgui_stdlib.cpp

    # SQLite (C)
    external/sqlite/sqlite3.c
)

add_executable(BudgetApp ${SOURCES})
target_link_libraries(BudgetApp glfw OpenGL::GL dl pthread)
