#pragma once
#include <string>
#include <ctime>
#include "core_logic.h"


//UI helpers

Account_type account_type_from_dropdown(int current_item);
int combo_index_from_account_type(Account_type t);
Account_type account_type_from_string(const char* s);
bool account_type_is_asset(Account_type t);
int compounding_frequency_from_index(int combo_index);
int compounding_index_from_frequency(int frequency);

float cents_to_dollars(int cents);

Transaction_type deposit_transaction_type_from_dropdown(int current_item_deposit);
Transaction_type withdrawal_transaction_type_from_dropdown(int current_item_withdrawal);

Transaction_category_need transaction_category_need_from_dropdown(int current_item_need);
Transaction_category_want transaction_category_want_from_dropdown(int current_item_want);

Transaction_info create_transaction_info(int account_id, int transaction_amount, Transaction_type type_of_transaction,
                                         Transaction_category_need transaction_category_need, Transaction_category_want transaction_category_want,
                                          std::string transaction_name, std::string note, int account_previous_amount, int account_new_amount);



//Database helpers

Transaction_type transaction_type_from_string(const char* type_text);
Transaction_category_need transaction_category_need_from_string(const char* category_text);
Transaction_category_want transaction_category_want_from_string(const char* category_text);


//storage.cpp helpers

const char* account_type_to_string(Account_type account_type);
const char* transaction_type_to_string(Transaction_type type_of_transaction);
const char* transaction_category_need_to_string(Transaction_category_need transaction_category_need);
const char* transaction_category_want_to_string(Transaction_category_want transaction_category_want);