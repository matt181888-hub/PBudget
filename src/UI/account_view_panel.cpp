#include "account_view_panel.h"
#include "../future_app_state.h"
#include "../../external/imgui/imgui.h"
#include "../helpers.h"
#include "transaction_form.h"
#include "latest_transactions_table.h"

void draw_account_view_panel(App_state& state, Controller& controller, float right_pane_width, ImFont* font_large)
{
    const auto& acc = state.wallet[state.selected_account_index];

    // Top row: account name + balance (left), monthly summary (right)
    const float summary_offset = (right_pane_width * 0.45f);
    const float y0 = ImGui::GetCursorPosY();
    ImGui::BeginGroup();
    ImGui::PushFont(font_large);
    ImGui::TextUnformatted(acc.account_name.c_str());
    if (acc.is_asset)
        ImGui::Text("%.2f$", cents_to_dollars(acc.money_amount));
    else
        ImGui::Text("%.2f$ owed", cents_to_dollars(acc.money_amount));
    ImGui::PopFont();
    ImGui::EndGroup();
    const float y_after_left = ImGui::GetCursorPosY();

    // Monthly summary: default to current month, allow prev/next; reset when switching account
    static int display_year = -1;
    static int display_month = -1;
    static int last_summary_account_index = -1;
    if (last_summary_account_index != state.selected_account_index) {
        last_summary_account_index = state.selected_account_index;
        display_year = -1;
        display_month = -1;
    }
    std::time_t now_t = std::time(nullptr);
    std::tm* now_lt = std::localtime(&now_t);
    if (display_year < 0 || display_month < 0) {
        display_year = now_lt->tm_year + 1900;
        display_month = now_lt->tm_mon + 1;
    }
    ImGui::SameLine(summary_offset);
    ImGui::SetCursorPosY(y0);
    ImGui::BeginGroup();
    std::tm start_tm = {};
    start_tm.tm_year = display_year - 1900;
    start_tm.tm_mon = display_month - 1;
    start_tm.tm_mday = 1;
    start_tm.tm_isdst = -1;
    std::time_t month_start = std::mktime(&start_tm);
    std::tm end_tm = {};
    end_tm.tm_year = display_year - 1900;
    end_tm.tm_mon = display_month;
    end_tm.tm_mday = 1;
    end_tm.tm_isdst = -1;
    std::time_t month_end = std::mktime(&end_tm);




    specific_range_of_transactions_info range_info = controller.get_monthly_summary(acc.account_id, month_start, month_end);




    const char* month_names[] = { "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December" };
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.f, 2.f));
    if (ImGui::Button("<", ImVec2(22.f, 0.f)))
    {
        if (display_month <= 1) { display_month = 12; display_year--; }
        else { display_month--; }
    }
    ImGui::SameLine();
    ImGui::Text("%s %d", display_month >= 1 && display_month <= 12 ? month_names[display_month - 1] : "?", display_year);
    ImGui::SameLine();
    if (ImGui::Button(">", ImVec2(22.f, 0.f)))
    {
        if (display_month >= 12) { display_month = 1; display_year++; }
        else { display_month++; }
    }
    ImGui::PopStyleVar();
    ImGui::Text("In:   %.2f$", cents_to_dollars(range_info.money_in));
    ImGui::Text("Out:  %.2f$", -cents_to_dollars(range_info.money_out));
    ImGui::Text("Remaining: %.2f$", cents_to_dollars(range_info.money_remaining));
    ImGui::EndGroup();

    ImGui::SetCursorPosX(0.f);
    ImGui::SetCursorPosY(y_after_left);
    ImGui::Spacing();

    if (ImGui::Button("Create Transaction(s)"))
        state.create_transaction_open = !state.create_transaction_open;

    if (state.create_transaction_open)
    {
        draw_transaction_form(state, controller);
    }
    else
    {
        ImGui::Spacing();
        if (ImGui::Button("Close account view"))
            state.selected_account_index = -1;
    }

    draw_latest_transactions_table(state, controller);

    ImGui::Spacing();
    static bool all_txn_modal_open = true;
    if (ImGui::Button("View all transactions"))
    {
        all_txn_modal_open = true;
        ImGui::OpenPopup("All Transactions");
    }
    if (ImGui::BeginPopupModal("All Transactions", &all_txn_modal_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        const auto& all_txns = controller.get_transactions(acc.account_id);
        if (all_txns.empty())
            ImGui::TextUnformatted("No transactions.");
        else
        {
            ImGui::BeginChild("TxnList", ImVec2(450, 350), true);
            for (int i = static_cast<int>(all_txns.size()) - 1; i >= 0; --i)
            {
                const auto& t = all_txns[i];
                char date_buf[32];
                std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M", std::localtime(&t.ymd));
                const float dollars = cents_to_dollars(t.transaction_amount);
                const char* sign = t.transaction_amount >= 0 ? "+" : "";
                ImGui::Text("%s  %s%.2f  %s  %s", t.transaction_name.c_str(), sign, dollars, transaction_type_to_string(t.type_of_transaction), date_buf);
                if (!t.note.empty())
                    ImGui::TextWrapped("  %s", t.note.c_str());
            }
            ImGui::EndChild();
        }
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
