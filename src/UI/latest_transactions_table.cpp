#include "latest_transactions_table.h"
#include "../future_app_state.h"
#include "../../external/imgui/imgui.h"
#include "../helpers.h"


void draw_latest_transactions_table(App_state& state, Controller& controller)
{
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Latest transactions");
    const auto& acc = state.wallet[state.selected_account_index];
    const auto& txns = controller.get_transactions(acc.account_id);
    const int n = static_cast<int>(txns.size());
    const int show_count = std::min(10, n);
    if (show_count == 0)
        ImGui::TextUnformatted("No transactions yet.");
    else
    {
        if (ImGui::BeginTable("LatestTransactions", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableHeadersRow();
            for (int i = 0; i < show_count; ++i)
            {
                const auto& t = txns[n - 1 - i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(t.transaction_name.c_str());
                ImGui::TableNextColumn();
                char amount_buf[24];
                const float dollars = cents_to_dollars(t.transaction_amount);
                const char* sign = t.transaction_amount >= 0 ? "+" : "";
                std::snprintf(amount_buf, sizeof(amount_buf), "%s%.2f", sign, dollars);
                ImGui::TextUnformatted(amount_buf);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(transaction_type_to_string(t.type_of_transaction));
                ImGui::TableNextColumn();
                char date_buf[32];
                std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M", std::localtime(&t.ymd));
                ImGui::TextUnformatted(date_buf);
                ImGui::TableNextColumn();
                ImGui::PushID(t.transaction_id);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.25f, 0.25f, 1.0f));
                if (ImGui::Button("x"))
                {
                    controller.delete_transaction(t.transaction_id, acc.account_id);
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }
}