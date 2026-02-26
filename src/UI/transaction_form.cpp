#include "transaction_form.h"
#include "../future_app_state.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/misc/cpp/imgui_stdlib.h"
#include "../helpers.h"
#include "../app_controller.h"

void draw_transaction_form(App_state& state, Controller& controller)
{
    const auto& acc = state.wallet[state.selected_account_index];
    ImGui::Separator();
    ImGui::Text("New Transaction");
    ImGui::Spacing();

    static std::string input_transaction_name;
    ImGui::InputTextWithHint("##Transaction_Name", "Enter transaction name...", &input_transaction_name);

    static float input_float_transaction_amount = 0;
    static int input_int_transaction_amount = 0;
    ImGui::InputFloat("##Money_Label", &input_float_transaction_amount, 0.0f, 0.0f, "%.2f");
    input_int_transaction_amount = static_cast<int>(input_float_transaction_amount * 100);

    static int transaction_mode = 0; // 0=Withdrawal, 1=Deposit, 2=Transfer
    ImGui::RadioButton("Withdrawal", &transaction_mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Deposit", &transaction_mode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Transfer", &transaction_mode, 2);

    Transaction_category_need input_transaction_category_need = Transaction_category_need::Other;
    Transaction_category_want input_transaction_category_want = Transaction_category_want::Other;
    Transaction_type input_transaction_type = Transaction_type::Other;
    static int transfer_target_wallet_index = -1;

    if (transaction_mode == 2)
    {
        input_transaction_type = Transaction_type::Internal_transfer;

        std::string preview = (transfer_target_wallet_index >= 0 &&
                               transfer_target_wallet_index < static_cast<int>(state.wallet.size()) &&
                               transfer_target_wallet_index != state.selected_account_index)
            ? state.wallet[transfer_target_wallet_index].account_name
            : "Select account...";

        if (ImGui::BeginCombo("Transfer to##transfer_target", preview.c_str()))
        {
            for (int i = 0; i < static_cast<int>(state.wallet.size()); i++)
            {
                if (i == state.selected_account_index) continue;
                bool is_selected = (transfer_target_wallet_index == i);
                if (ImGui::Selectable(state.wallet[i].account_name.c_str(), is_selected))
                {
                    transfer_target_wallet_index = i;
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    else if (transaction_mode == 1)
    {
        static int current_item_deposit = 0;
        const char* items_deposit = "Income\0Gift\0Dividends\0Other\0\0";
        ImGui::Combo("Transaction type##deposit", &current_item_deposit, items_deposit);

        input_transaction_type = deposit_transaction_type_from_dropdown(current_item_deposit);
    }
    else
    {
        static int current_item_withdrawal = 0;
        const char* items_withdrawal = "Need\0Want\0Savings\0Other\0\0";
        ImGui::Combo("Transaction type##withdrawal", &current_item_withdrawal, items_withdrawal);

        input_transaction_type = withdrawal_transaction_type_from_dropdown(current_item_withdrawal);
    }

    if (input_transaction_type == Transaction_type::Need)
    {
        static int current_item_need = 0;
        const char* items_need = "Housing\0Food\0Transportation\0Utilities\0Healthcare\0Debt\0Dependants\0Other\0\0";
        ImGui::Combo("Transaction category##need", &current_item_need, items_need);

        input_transaction_category_need = transaction_category_need_from_dropdown(current_item_need);
    }
    else if (input_transaction_type == Transaction_type::Want)
    {
        static int current_item_want = 0;
        const char* items_want = "Shopping\0Entertainment\0Eating_out\0Travel\0Leisure\0Gifts\0Other\0\0";
        ImGui::Combo("Transaction category##want", &current_item_want, items_want);

        input_transaction_category_want = transaction_category_want_from_dropdown(current_item_want);
    }

    static std::string input_optional_description;
    ImGui::InputTextWithHint("##Optional_description", "Optional description...", &input_optional_description);

    if (ImGui::Button("Confirm"))
    {
        if (transaction_mode == 2)
        {
            bool valid_target = transfer_target_wallet_index >= 0 &&
                                transfer_target_wallet_index < static_cast<int>(state.wallet.size()) &&
                                transfer_target_wallet_index != state.selected_account_index;
            if (valid_target)
            {
                Transaction_info trans_info;
                trans_info.transaction_amount = input_int_transaction_amount;
                trans_info.type_of_transaction = Transaction_type::Internal_transfer;
                trans_info.transaction_name = input_transaction_name;
                trans_info.note = input_optional_description;
                trans_info.ymd = std::time(nullptr);

                int from_id = acc.account_id;
                int to_id = state.wallet[transfer_target_wallet_index].account_id;
                controller.create_internal_transfer(from_id, to_id, trans_info);

                input_transaction_name.clear();
                input_optional_description.clear();
                input_float_transaction_amount = 0;
                transfer_target_wallet_index = -1;
            }
        }
        else
        {
            int account_previous_amount = acc.money_amount;
            bool is_deposit = (transaction_mode == 1);
            int account_new_amount = balance_after_transaction(account_previous_amount, input_int_transaction_amount, is_deposit, acc.is_asset);
            int transaction_amount_stored = account_new_amount - account_previous_amount;

            Transaction_info trans_info = create_transaction_info(acc.account_id, transaction_amount_stored, input_transaction_type,
                                                                input_transaction_category_need, input_transaction_category_want,
                                                                input_transaction_name, input_optional_description, account_previous_amount,
                                                                account_new_amount);

            controller.create_transaction(acc.account_id, trans_info);
            input_transaction_name.clear();
            input_optional_description.clear();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Exit"))
    {
        state.create_transaction_open = false;
        input_transaction_name.clear();
        input_optional_description.clear();
        transfer_target_wallet_index = -1;
    }
}