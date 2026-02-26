#pragma once
#include "../future_app_state.h"
#include "../app_controller.h"

struct Sidebar_result
{
    bool exit_requested = false;
};

Sidebar_result draw_sidebar(App_state& state, Controller& controller, float left_pane_width);
