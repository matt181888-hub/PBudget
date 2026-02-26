#pragma once
#include "core_logic.h"
#include <map>
#include <vector>
extern "C"{
    #include "../external/sqlite/sqlite3.h"
}


struct Account_info
{
    int account_id;
    int money_amount;
    int initial_money_amount;
    std::string account_name;
    std::string account_type;
    bool is_asset;
    int interest_rate;
    int compounding_frequency;
    int principal;
    int term;
    int monthly_payment;
    int remaining_balance;
    int remaining_term;
    int remaining_interest;
    int remaining_principal;
    int remaining_total;
    int credit_limit;
    int minimum_payment;
};

struct specific_range_of_transactions_info
{
    int money_in = 0;
    int money_out = 0;
    int money_remaining = 0;
};

class Storage
{ 
    public:
        //constructors
        Storage();
        ~Storage();

        //methods
        void save_account_info(Account &acc);
        void save_transaction_info(int account_id, Transaction_info &trans);
        void load_transactions(int account_id);   // refresh one account's list in cache
        void load_all_transactions();             // load all transactions at startup
        void delete_transaction(int transaction_id, int account_id);
        std::vector<Transaction_info> get_monthly_information(int account_id, std::time_t start_time, std::time_t end_time);
        void modify_account_in_storage(int account_id, std::string new_account_name, Account_type new_type_of_account, int new_money,
            int interest_rate, int compounding_frequency, int principal, int term, int monthly_payment, 
            int remaining_balance, int remaining_term, int remaining_interest, int remaining_principal, 
            int remaining_total, int credit_limit, int minimum_payment);
        Transaction_info get_transaction_info_from_stmt(sqlite3_stmt* stmt);
        void delete_account(int account_id);
        void save_internal_transfer(int account_id_from, int account_id_to, Transaction_info &trans);
        
        std::vector<Account_info> load_accounts();
        const std::vector<Transaction_info>& get_transactions(int account_id);
        specific_range_of_transactions_info get_specific_range_of_transactions_info(std::vector<Transaction_info> &range_of_transactions);

        bool empty();

    private:
        sqlite3 *db = nullptr;
        std::vector<Account_info> accounts_vec;
        std::map<int, std::vector<Transaction_info>> transactions_by_account;
};