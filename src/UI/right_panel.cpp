#include "right_panel.h"
#include "../future_app_state.h"
#include "../../external/imgui/imgui.h"
#include "create_account_panel.h"
#include "modify_account_panel.h"
#include "account_view_panel.h"
#include "../app_controller.h"

void draw_right_panel(App_state& state, Controller& controller, float right_pane_width, ImFont* font_large)
{
    ImGui::BeginChild("RightPane", ImVec2(right_pane_width, 0), true);

    if (state.new_account_open)
        draw_create_account_panel(state, controller);
    else if (state.modify_account_index >= 0 && state.modify_account_index < (int)state.wallet.size())
        draw_modify_account_panel(state, controller);
    else if (state.selected_account_index >= 0 && state.selected_account_index < (int)state.wallet.size())
        draw_account_view_panel(state, controller, right_pane_width, font_large);
    else
    {
        ImGui::Text("Welcome to MyBudget!");
        ImGui::Spacing();
        ImGui::TextWrapped("Create an account, open your wallet, then select an account to view details and add transactions.");
    }

    ImGui::EndChild();
}
