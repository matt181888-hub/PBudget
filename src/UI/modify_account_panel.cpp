#include "modify_account_panel.h"
#include "../future_app_state.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/misc/cpp/imgui_stdlib.h"
#include "../helpers.h"
#include "../app_controller.h"
void draw_modify_account_panel(App_state& state, Controller& controller)
{
    ImGuiIO& io = ImGui::GetIO();
    const auto& acc = state.wallet[state.modify_account_index];
    static std::string modify_account_name;
    static float modify_balance_float = 0.f;
    static int modify_type_combo = 0;
    static int last_modify_index = -1;
    static float modify_interest_rate = 0.f;
    static int modify_compounding = 0;
    static int modify_principal = 0, modify_term = 0, modify_monthly_payment = 0;
    static int modify_remaining_balance = 0, modify_remaining_term = 0, modify_remaining_interest = 0;
    static int modify_remaining_principal = 0, modify_remaining_total = 0, modify_credit_limit = 0, modify_minimum_payment = 0;
    static float mod_principal_f = 0.f, mod_monthly_f = 0.f, mod_credit_f = 0.f, mod_min_f = 0.f;
    static float mod_rem_bal_f = 0.f;
    static int modify_compounding_index = 1;
    if (last_modify_index != state.modify_account_index)
    {
        last_modify_index = state.modify_account_index;
        modify_account_name = acc.account_name;
        modify_balance_float = acc.money_amount / 100.0f;
        modify_type_combo = combo_index_from_account_type(account_type_from_string(acc.account_type.c_str()));
        modify_interest_rate = acc.interest_rate / 100.0f;
        modify_compounding = acc.compounding_frequency;
        modify_compounding_index = compounding_index_from_frequency(acc.compounding_frequency);
        modify_principal = acc.principal;
        modify_term = acc.term;
        modify_monthly_payment = acc.monthly_payment;
        modify_remaining_balance = acc.remaining_balance;
        modify_remaining_term = acc.remaining_term;
        modify_remaining_interest = acc.remaining_interest;
        modify_remaining_principal = acc.remaining_principal;
        modify_remaining_total = acc.remaining_total;
        modify_credit_limit = acc.credit_limit;
        modify_minimum_payment = acc.minimum_payment;
        mod_principal_f = acc.principal / 100.0f;
        mod_monthly_f = acc.monthly_payment / 100.0f;
        mod_credit_f = acc.credit_limit / 100.0f;
        mod_min_f = acc.minimum_payment / 100.0f;
        mod_rem_bal_f = acc.remaining_balance / 100.0f;
    }
    ImGui::Text("Modify account");
    ImGui::Spacing();
    ImGui::Text("Account name");
    ImGui::InputTextWithHint("##ModifyAccount_Name", "Account name...", &modify_account_name);
    ImGui::Text("Account type");
    const char* type_items = "Checking\0Savings\0Investments\0Credit Card\0Loan\0Mortgage\0Other\0\0";
    ImGui::Combo("##ModifyType", &modify_type_combo, type_items);
    const bool modify_is_asset = account_type_is_asset(account_type_from_dropdown(modify_type_combo));
    ImGui::Text(modify_is_asset ? "Current balance" : "Current balance owed");
    ImGui::InputFloat("##ModifyBalance", &modify_balance_float, 0.0f, 0.0f, "%.2f");

    if (modify_type_combo == 1 || modify_type_combo == 2)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Optional details");
        ImGui::Text("Interest rate");
        ImGui::InputFloat("##mod_asset_ir", &modify_interest_rate, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Interest is applied:");
        ImGui::Combo("##mod_asset_comp", &modify_compounding_index, "Daily\0Monthly\0Annually\0\0");
        modify_compounding = compounding_frequency_from_index(modify_compounding_index);
    }
    else if (modify_type_combo == 3)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Optional details");
        ImGui::Text("Interest rate");
        ImGui::InputFloat("##mod_cc_ir", &modify_interest_rate, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Interest is applied:");
        ImGui::Combo("##mod_cc_comp", &modify_compounding_index, "Daily\0Monthly\0Annually\0\0");
        ImGui::Text("Credit limit");
        ImGui::InputFloat("##mod_cc_cl", &mod_credit_f, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Minimum payment");
        ImGui::InputFloat("##mod_cc_min", &mod_min_f, 0.0f, 0.0f, "%.2f");
        modify_compounding = compounding_frequency_from_index(modify_compounding_index);
        modify_principal = 0;
        modify_term = 0;
        modify_monthly_payment = 0;
        modify_remaining_balance = static_cast<int>(modify_balance_float * 100);
        modify_remaining_term = 0;
        modify_remaining_principal = modify_remaining_balance;
        modify_remaining_total = modify_remaining_balance;
        modify_credit_limit = static_cast<int>(mod_credit_f * 100);
        modify_minimum_payment = static_cast<int>(mod_min_f * 100);
    }
    else if (modify_type_combo == 4 || modify_type_combo == 5 || modify_type_combo == 6)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Optional details");
        ImGui::Text("Interest rate");
        ImGui::InputFloat("##mod_liab_ir", &modify_interest_rate, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Interest is applied:");
        ImGui::Combo("##mod_liab_comp", &modify_compounding_index, "Daily\0Monthly\0Annually\0\0");
        ImGui::Text("Principal");
        ImGui::InputFloat("##mod_liab_pr", &mod_principal_f, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Term (months)");
        ImGui::InputInt("##mod_liab_term", &modify_term, 0, 0);
        ImGui::Text("Monthly payment");
        ImGui::InputFloat("##mod_liab_mp", &mod_monthly_f, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Remaining balance");
        ImGui::InputFloat("##mod_liab_rb", &mod_rem_bal_f, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Minimum payment");
        ImGui::InputFloat("##mod_liab_min", &mod_min_f, 0.0f, 0.0f, "%.2f");
        modify_compounding = compounding_frequency_from_index(modify_compounding_index);
        modify_principal = static_cast<int>(mod_principal_f * 100);
        modify_monthly_payment = static_cast<int>(mod_monthly_f * 100);
        modify_remaining_balance = static_cast<int>(mod_rem_bal_f * 100);
        modify_remaining_principal = modify_remaining_balance;
        modify_remaining_total = modify_remaining_balance;
        modify_credit_limit = 0;
        modify_minimum_payment = static_cast<int>(mod_min_f * 100);
    }

    ImGui::Spacing();
    if (ImGui::Button("Save"))
    {
        
        Account_type new_type = account_type_from_dropdown(modify_type_combo);
        int new_money_cents = static_cast<int>(modify_balance_float * 100);
        int ir = static_cast<int>(modify_interest_rate * 100);
        int cp = modify_compounding;
        int pr = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_principal : 0;
        int tm = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_term : 0;
        int mp = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_monthly_payment : 0;
        int rb = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_remaining_balance : 0;
        int rt = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_remaining_term : 0;
        int ri = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_remaining_interest : 0;
        int rp = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_remaining_principal : 0;
        int rtot = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_remaining_total : 0;
        int cl = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_credit_limit : 0;
        int minp = (modify_type_combo >= 3 && modify_type_combo <= 6) ? modify_minimum_payment : 0;
        controller.modify_account(acc.account_id, modify_account_name, new_type, new_money_cents,
            ir, cp, pr, tm, mp, rb, rt, ri, rp, rtot, cl, minp);
        last_modify_index = -1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        state.modify_account_index = -1;
        last_modify_index = -1;
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.25f, 0.25f, 1.0f));
    if (ImGui::Button("Delete account"))
    {
        ImGui::SetNextWindowSize(ImVec2(480.f, 0.f));  // 0 = auto height
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("Confirm delete account");
    }
    ImGui::PopStyleColor(3);
    if (ImGui::BeginPopupModal("Confirm delete account", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("Delete this account? All transactions for this account will be permanently removed.");
        ImGui::Spacing();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.25f, 0.25f, 1.0f));
        if (ImGui::Button("Delete"))
        {
            controller.delete_account(acc.account_id);
            last_modify_index = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);
        ImGui::EndPopup();
    }

}