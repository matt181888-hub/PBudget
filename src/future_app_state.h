#pragma once
#include <vector>
#include "storage.h"

struct App_state
{
    bool new_account_open = false;
    bool wallet_open = false;
    int selected_account_index = -1;
    int modify_account_index = -1;
    bool create_transaction_open = false;
    std::vector<Account_info> wallet;
};