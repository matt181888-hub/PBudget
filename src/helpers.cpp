#include "helpers.h"

// Transaction UI helpers

Transaction_type deposit_transaction_type_from_dropdown(int current_item_deposit)
{
    if (current_item_deposit == 0) return Transaction_type::Income;
    else if (current_item_deposit == 1) return Transaction_type::Gift;
    else if (current_item_deposit == 2) return Transaction_type::Dividends;
    else return Transaction_type::Other;
}

Transaction_type withdrawal_transaction_type_from_dropdown(int current_item_withdrawal)
{
    if (current_item_withdrawal == 0) return Transaction_type::Need;
    else if (current_item_withdrawal == 1) return Transaction_type::Want;
    else if (current_item_withdrawal == 2) return Transaction_type::Savings;
    else return Transaction_type::Other;
}

Transaction_category_need transaction_category_need_from_dropdown(int current_item_need)
{
    if (current_item_need == 0) return Transaction_category_need::Housing;
    else if (current_item_need == 1) return Transaction_category_need::Food;
    else if (current_item_need == 2) return Transaction_category_need::Transportation;
    else if (current_item_need == 3) return Transaction_category_need::Utilities;
    else if (current_item_need == 4) return Transaction_category_need::Healthcare;
    else if (current_item_need == 5) return Transaction_category_need::Debt;
    else if (current_item_need == 6) return Transaction_category_need::Dependants;
    else return Transaction_category_need::Other;
}

Transaction_category_want transaction_category_want_from_dropdown(int current_item_want)
{
    if (current_item_want == 0) return Transaction_category_want::Shopping;
    else if (current_item_want == 1) return Transaction_category_want::Entertainment;
    else if (current_item_want == 2) return Transaction_category_want::Eating_out;
    else if (current_item_want == 3) return Transaction_category_want::Travel;
    else if (current_item_want == 4) return Transaction_category_want::Leisure;
    else if (current_item_want == 5) return Transaction_category_want::Gifts;
    else return Transaction_category_want::Other;
}

Transaction_info create_transaction_info(int account_id, int transaction_amount, Transaction_type type_of_transaction,
                                         Transaction_category_need transaction_category_need, Transaction_category_want transaction_category_want,
                                          std::string transaction_name, std::string note, int account_previous_amount, int account_new_amount)
{
    Transaction_info trans_info;
    trans_info.account_id = account_id;
    trans_info.transaction_amount = transaction_amount;
    trans_info.type_of_transaction = type_of_transaction;
    trans_info.transaction_category_need = transaction_category_need;
    trans_info.transaction_category_want = transaction_category_want;
    trans_info.transaction_name = transaction_name;
    trans_info.note = note;
    trans_info.account_previous_amount = account_previous_amount;
    trans_info.account_new_amount = account_new_amount;
    trans_info.ymd = std::time(nullptr);
    return trans_info;
}

const char* transaction_type_to_string(Transaction_type type_of_transaction)
{
    switch (type_of_transaction)
    {
        case Transaction_type::Need: return "Need";
        case Transaction_type::Want: return "Want";
        case Transaction_type::Savings: return "Savings";
        case Transaction_type::Internal_transfer: return "Transfer";
        case Transaction_type::Income: return "Income";
        case Transaction_type::Gift: return "Gift";
        case Transaction_type::Dividends: return "Dividends";
        default: return "Other";
    }
}

const char* transaction_category_need_to_string(Transaction_category_need transaction_category_need)
{
    switch (transaction_category_need)
    {
        case Transaction_category_need::Housing: return "Housing";
        case Transaction_category_need::Food: return "Food";
        case Transaction_category_need::Transportation: return "Transportation";
        case Transaction_category_need::Utilities: return "Utilities";
        case Transaction_category_need::Healthcare: return "Healthcare";
        case Transaction_category_need::Debt: return "Debt";
        case Transaction_category_need::Dependants: return "Dependants";
        default: return "Other";
    }
}

const char* transaction_category_want_to_string(Transaction_category_want transaction_category_want)
{
    switch (transaction_category_want)
    {
        case Transaction_category_want::Shopping: return "Shopping";
        case Transaction_category_want::Entertainment: return "Entertainment";
        case Transaction_category_want::Eating_out: return "Eating_out";
        case Transaction_category_want::Travel: return "Travel";
        case Transaction_category_want::Leisure: return "Leisure";
        case Transaction_category_want::Gifts: return "Gifts";
        default: return "Other";
    }
}

// Account UI helpers
Account_type account_type_from_dropdown(int current_item)
{
    if (current_item == 0) return Account_type::checking;
    else if (current_item == 1) return Account_type::savings;
    else if (current_item == 2) return Account_type::investments;
    else if (current_item == 3) return Account_type::credit_card;
    else if (current_item == 4) return Account_type::loan;
    else if (current_item == 5) return Account_type::mortgage;
    else return Account_type::other;
}

int combo_index_from_account_type(Account_type t)
{
    switch (t)
    {
        case Account_type::checking:   return 0;
        case Account_type::savings:   return 1;
        case Account_type::investments: return 2;
        case Account_type::credit_card: return 3;
        case Account_type::loan:      return 4;
        case Account_type::mortgage:  return 5;
        case Account_type::other:     return 6;
        default:                      return 0;
    }
}

Account_type account_type_from_string(const char* s)
{
    if (!s) return Account_type::checking;
    if (std::strcmp(s, "Checking") == 0) return Account_type::checking;
    if (std::strcmp(s, "Savings") == 0) return Account_type::savings;
    if (std::strcmp(s, "Investments") == 0) return Account_type::investments;
    if (std::strcmp(s, "Credit Card") == 0) return Account_type::credit_card;
    if (std::strcmp(s, "Loan") == 0) return Account_type::loan;
    if (std::strcmp(s, "Mortgage") == 0) return Account_type::mortgage;
    if (std::strcmp(s, "Other") == 0) return Account_type::other;
    return Account_type::checking;
}

bool account_type_is_asset(Account_type t)
{
    return t == Account_type::checking || t == Account_type::savings || t == Account_type::investments;
}

int compounding_frequency_from_index(int combo_index)
{
    if (combo_index == 0) return 365;
    if (combo_index == 1) return 12;
    return 1;
}

int compounding_index_from_frequency(int frequency)
{
    if (frequency == 365) return 0;
    if (frequency == 12) return 1;
    return 2;
}

float cents_to_dollars(int cents)
{
    return static_cast<float>(cents) / 100.0f;
}

// Transaction database helpers
Transaction_type transaction_type_from_string(const char* type_text)
{
    if (!type_text) return Transaction_type::Other;
    if (std::strcmp(type_text, "Need") == 0) return Transaction_type::Need;
    else if (std::strcmp(type_text, "Want") == 0) return Transaction_type::Want;
    else if (std::strcmp(type_text, "Savings") == 0) return Transaction_type::Savings;
    else if (std::strcmp(type_text, "Internal_transfer") == 0) return Transaction_type::Internal_transfer;
    else if (std::strcmp(type_text, "Income") == 0) return Transaction_type::Income;
    else if (std::strcmp(type_text, "Gift") == 0) return Transaction_type::Gift;
    else if (std::strcmp(type_text, "Dividends") == 0) return Transaction_type::Dividends;
    else return Transaction_type::Other;
}

Transaction_category_need transaction_category_need_from_string(const char* category_text)
{
    if (!category_text) return Transaction_category_need::Other;
    if (std::strcmp(category_text, "Housing") == 0) return Transaction_category_need::Housing;
    else if (std::strcmp(category_text, "Food") == 0) return Transaction_category_need::Food;
    else if (std::strcmp(category_text, "Transportation") == 0) return Transaction_category_need::Transportation;
    else if (std::strcmp(category_text, "Utilities") == 0) return Transaction_category_need::Utilities;
    else if (std::strcmp(category_text, "Healthcare") == 0) return Transaction_category_need::Healthcare;
    else if (std::strcmp(category_text, "Debt") == 0) return Transaction_category_need::Debt;
    else if (std::strcmp(category_text, "Dependants") == 0) return Transaction_category_need::Dependants;
    else return Transaction_category_need::Other;
}

Transaction_category_want transaction_category_want_from_string(const char* category_text)
{
    if (!category_text) return Transaction_category_want::Other;
    if (std::strcmp(category_text, "Shopping") == 0) return Transaction_category_want::Shopping;
    else if (std::strcmp(category_text, "Entertainment") == 0) return Transaction_category_want::Entertainment;
    else if (std::strcmp(category_text, "Eating_out") == 0) return Transaction_category_want::Eating_out;
    else if (std::strcmp(category_text, "Travel") == 0) return Transaction_category_want::Travel;
    else if (std::strcmp(category_text, "Leisure") == 0) return Transaction_category_want::Leisure;
    else if (std::strcmp(category_text, "Gifts") == 0) return Transaction_category_want::Gifts;
    else return Transaction_category_want::Other;
}



//storage.cpp helpers
const char* account_type_to_string(Account_type account_type)
{
    switch (account_type)
    {
        case Account_type::checking: return "Checking";
        case Account_type::savings: return "Savings";
        case Account_type::investments: return "Investments";
        case Account_type::credit_card: return "Credit Card";
        case Account_type::loan: return "Loan";
        case Account_type::mortgage: return "Mortgage";
        case Account_type::other: return "Other";
        default: return "";
    }
}

