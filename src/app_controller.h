#pragma once
#include "future_app_state.h"
#include "storage.h"

class Controller
{
    public:
        Controller(App_state& state, Storage& myDB);

        // write actions
        void create_account(Account& account);
        void modify_account(int account_id, const std::string& name, Account_type type,
                            int money_cents, int ir, int cp, int pr, int tm, int mp,
                            int rb, int rt, int ri, int rp, int rtot, int cl, int minp);
        void delete_account(int account_id);
        void create_transaction(int account_id, Transaction_info& trans);
        void create_internal_transfer(int account_id_from, int account_id_to, Transaction_info& trans);
        void delete_transaction(int transaction_id, int account_id);
  
        void reload_wallet();

        // read queries
        const std::vector<Transaction_info>& get_transactions(int account_id);
        specific_range_of_transactions_info get_monthly_summary(int account_id,
                                                                std::time_t start,
                                                                std::time_t end);

    private:
        App_state& state;
        Storage& db;
};

