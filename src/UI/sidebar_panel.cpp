#include "sidebar_panel.h"
#include "../future_app_state.h"
#include "../../external/imgui/imgui.h"
#include "../app_controller.h"

Sidebar_result draw_sidebar(App_state& state, Controller& controller, float left_pane_width)
{
    Sidebar_result result;

    ImGui::BeginChild("LeftPane", ImVec2(left_pane_width, 0), true);
    ImGui::SetCursorPosX((left_pane_width - ImGui::CalcTextSize("MyBudget").x) * 0.5f);
    ImGui::Text("MyBudget");
    ImGui::Spacing();

    {
        const char* lbl = "Create a new account!";
        float w = ImGui::CalcTextSize(lbl).x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SetCursorPosX((left_pane_width - w) * 0.5f);
        if (ImGui::Button(lbl))
        {
            state.new_account_open = !state.new_account_open;
            if (state.new_account_open)
            {
                state.modify_account_index = -1;
                state.selected_account_index = -1;
            }
        }
    }

    if (!state.wallet.empty())
    {
        const char* open_wallet_lbl = "Open wallet";
        float w = ImGui::CalcTextSize(open_wallet_lbl).x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SetCursorPosX((left_pane_width - w) * 0.5f);
        if (ImGui::Button(open_wallet_lbl))
            state.wallet_open = !state.wallet_open;
    }

    if (state.wallet_open)
    {
        ImGui::Separator();
        ImGui::Text("Accounts");
        const float settings_btn_w = 36.f;
        const float account_btn_h = ImGui::GetFrameHeight();
        for (int i = 0; i < (int)state.wallet.size(); ++i)
        {
            ImGui::PushID(i);
            const auto& acc = state.wallet[i];
            const char* account_name = acc.account_name.c_str();
            bool is_selected = (state.selected_account_index == i);
            const float btn_w = left_pane_width - ImGui::GetStyle().WindowPadding.x * 2 - (settings_btn_w + ImGui::GetStyle().ItemSpacing.x);
            if (is_selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            if (ImGui::Button(account_name, ImVec2(btn_w, 0)))
            {
                state.new_account_open = false;
                state.modify_account_index = -1;
                state.selected_account_index = (state.selected_account_index == i) ? -1 : i;
                state.create_transaction_open = false;
            }
            if (is_selected)
                ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.40f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.45f, 0.50f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.50f, 0.55f, 1.0f));
            if (ImGui::Button("...", ImVec2(settings_btn_w, account_btn_h)))
            {
                if (state.modify_account_index == i)
                    state.modify_account_index = -1;
                else
                {
                    state.modify_account_index = i;
                    state.new_account_open = false;
                    state.selected_account_index = -1;
                }
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    {
        const char* exit_lbl = "Exit";
        float w = ImGui::CalcTextSize(exit_lbl).x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SetCursorPosX((left_pane_width - w) * 0.5f);
        if (ImGui::Button(exit_lbl))
            result.exit_requested = true;
    }
    ImGui::EndChild();

    return result;
}
