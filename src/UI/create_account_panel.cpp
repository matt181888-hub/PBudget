#include "create_account_panel.h"
#include "../future_app_state.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/misc/cpp/imgui_stdlib.h"
#include "../helpers.h"
#include "../app_controller.h"

void draw_create_account_panel(App_state& state, Controller& controller)
{
    ImGui::Text("Create a new account");
    ImGui::Spacing();

    ImGui::Text("Account name");
    static std::string input_account_name;
    ImGui::InputTextWithHint("##Account_Name", "Enter Account Name Here...", &input_account_name);

    ImGui::Text("Account type");
    static int current_item = 0;
    const char* items = "Checking\0Savings\0Investments\0Credit Card\0Loan\0Mortgage\0Other\0\0";
    ImGui::Combo("##AccountType", &current_item, items);

    Account_type input_account_type = account_type_from_dropdown(current_item);
    const bool create_is_asset = account_type_is_asset(input_account_type);

    ImGui::Text(create_is_asset ? "Starting balance" : "Current balance owed");
    static float input_float_account_money = 0.f;
    ImGui::InputFloat("##Money_Label", &input_float_account_money, 0.0f, 0.0f, "%.2f");

    static int input_account_money = 0;

    // Optional asset params (Savings, Investments only)
    if (current_item == 1 || current_item == 2)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Optional details (add later if you like)");
        static float create_interest_rate = 0.f;
        static int create_compounding_index = 1; // 0=Daily, 1=Monthly, 2=Annually
        ImGui::Text("Interest rate");
        ImGui::InputFloat("##InterestAsset", &create_interest_rate, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Interest is applied:");
        ImGui::Combo("##CompoundingAsset", &create_compounding_index, "Daily\0Monthly\0Annually\0\0");
        const int create_compounding_val = compounding_frequency_from_index(create_compounding_index);
        if (ImGui::Button("Confirm"))
        {
            input_account_money = static_cast<int>(input_float_account_money * 100);
            if (create_interest_rate > 0.f || create_compounding_val > 0)
            {
                Asset_parameters ap{};
                ap.asset_interest_rate = static_cast<int>(create_interest_rate * 100);
                ap.compounding_frequency = create_compounding_val;
                Account new_account(input_account_name, input_account_type, input_account_money, true, ap);

                controller.create_account(new_account);
            }
            else
            {
                Account new_account(input_account_name, input_account_type, input_account_money, true);

                controller.create_account(new_account);
            }
            create_interest_rate = 0.f;
            create_compounding_index = 1;
        }
    }
    // Credit Card only: interest rate, interest applied, credit limit, minimum payment
    else if (current_item == 3)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Optional details (add later if you like)");
        static float create_cc_interest = 0.f;
        static int create_cc_compounding_index = 1;
        static float create_credit_limit = 0.f, create_min_pmt = 0.f;
        ImGui::Text("Interest rate");
        ImGui::InputFloat("##InterestCC", &create_cc_interest, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Interest is applied:");
        ImGui::Combo("##CompoundingCC", &create_cc_compounding_index, "Daily\0Monthly\0Annually\0\0");
        ImGui::Text("Credit limit");
        ImGui::InputFloat("##CreditLimit", &create_credit_limit, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Minimum payment");
        ImGui::InputFloat("##MinPmtCC", &create_min_pmt, 0.0f, 0.0f, "%.2f");
        const int cc_compounding_val = compounding_frequency_from_index(create_cc_compounding_index);
        if (ImGui::Button("Confirm"))
        {
            input_account_money = static_cast<int>(input_float_account_money * 100);
            bool any_liab = (create_cc_interest > 0.f || create_credit_limit > 0.f || create_min_pmt > 0.f);
            if (any_liab)
            {
                Liability_parameters lp{};
                lp.liability_interest_rate = static_cast<int>(create_cc_interest * 100);
                lp.compounding_frequency = cc_compounding_val;
                lp.principal = 0;
                lp.term = 0;
                lp.monthly_payment = 0;
                lp.remaining_balance = input_account_money;
                lp.remaining_term = 0;
                lp.remaining_interest = 0;
                lp.remaining_principal = input_account_money;
                lp.remaining_total = input_account_money;
                lp.credit_limit = static_cast<int>(create_credit_limit * 100);
                lp.minimum_payment = static_cast<int>(create_min_pmt * 100);
                Account new_account(input_account_name, input_account_type, input_account_money, false, lp);
                controller.create_account(new_account);
            }
            else
            {
                Account new_account(input_account_name, input_account_type, input_account_money, false);
                controller.create_account(new_account);
            }
            create_cc_interest = 0.f;
            create_cc_compounding_index = 1;
            create_credit_limit = create_min_pmt = 0.f;
        }
    }
    // Loan, Mortgage, Other: no credit limit
    else if (current_item == 4 || current_item == 5 || current_item == 6)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Optional details (add later if you like)");
        static float create_liab_interest = 0.f;
        static int create_liab_compounding_index = 1;
        static float create_principal = 0.f, create_monthly_pmt = 0.f, create_min_pmt = 0.f, create_rem_bal = 0.f;
        static int create_term = 0;
        ImGui::Text("Interest rate");
        ImGui::InputFloat("##InterestLiab", &create_liab_interest, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Interest is applied:");
        ImGui::Combo("##CompoundingLiab", &create_liab_compounding_index, "Daily\0Monthly\0Annually\0\0");
        ImGui::Text("Principal (original amount)");
        ImGui::InputFloat("##Principal", &create_principal, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Term (months)");
        ImGui::InputInt("##Term", &create_term, 0, 0);
        ImGui::Text("Monthly payment");
        ImGui::InputFloat("##MonthlyPmt", &create_monthly_pmt, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Remaining balance");
        ImGui::InputFloat("##RemBal", &create_rem_bal, 0.0f, 0.0f, "%.2f");
        ImGui::Text("Minimum payment");
        ImGui::InputFloat("##MinPmtLiab", &create_min_pmt, 0.0f, 0.0f, "%.2f");
        const int liab_compounding_val = compounding_frequency_from_index(create_liab_compounding_index);
        if (ImGui::Button("Confirm"))
        {
            input_account_money = static_cast<int>(input_float_account_money * 100);
            bool any_liab = (create_liab_interest > 0.f || create_principal > 0.f || create_term > 0 ||
                create_monthly_pmt > 0.f || create_rem_bal > 0.f || create_min_pmt > 0.f);
            if (any_liab)
            {
                int rem_cents = create_rem_bal > 0.f ? static_cast<int>(create_rem_bal * 100) : input_account_money;
                Liability_parameters lp{};
                lp.liability_interest_rate = static_cast<int>(create_liab_interest * 100);
                lp.compounding_frequency = liab_compounding_val;
                lp.principal = static_cast<int>(create_principal * 100);
                lp.term = create_term;
                lp.monthly_payment = static_cast<int>(create_monthly_pmt * 100);
                lp.remaining_balance = rem_cents;
                lp.remaining_term = create_term;
                lp.remaining_interest = 0;
                lp.remaining_principal = rem_cents;
                lp.remaining_total = rem_cents;
                lp.credit_limit = 0;
                lp.minimum_payment = static_cast<int>(create_min_pmt * 100);
                Account new_account(input_account_name, input_account_type, input_account_money, false, lp);
                controller.create_account(new_account);
            }
            else
            {
                Account new_account(input_account_name, input_account_type, input_account_money, false);
                controller.create_account(new_account);
            }
            create_liab_interest = 0.f;
            create_liab_compounding_index = 1;
            create_principal = create_monthly_pmt = create_min_pmt = create_rem_bal = 0.f;
            create_term = 0;
        }
    }
    else
    {
        // Checking: no optional params
        if (ImGui::Button("Confirm"))
        {
            input_account_money = static_cast<int>(input_float_account_money * 100);
            Account new_account(input_account_name, input_account_type, input_account_money, true);

            controller.create_account(new_account);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Exit"))
        state.new_account_open = false;

}