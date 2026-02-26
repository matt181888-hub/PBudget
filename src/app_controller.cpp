#include "app_controller.h"
#include "future_app_state.h"
#include "storage.h"
#include <vector>


Controller::Controller(App_state& state, Storage& myDB) : state(state), db(myDB) {}


void Controller::create_account(Account& account)
{
    db.save_account_info(account);
    reload_wallet();
    state.new_account_open = false;
}

void Controller::modify_account(int account_id, const std::string& name, Account_type type,
                            int money_cents, int ir, int cp, int pr, int tm, int mp,
                            int rb, int rt, int ri, int rp, int rtot, int cl, int minp)
{
    db.modify_account_in_storage(account_id, name, type, money_cents, ir, cp, pr, tm, mp, rb, rt, ri, rp, rtot, cl, minp);
    state.modify_account_index = -1;
    reload_wallet();
}

void Controller::delete_account(int account_id)
{
    db.delete_account(account_id);
    state.selected_account_index = -1;
    state.modify_account_index = -1;
    db.load_all_transactions();
    reload_wallet();
}

void Controller::create_transaction(int account_id, Transaction_info& trans)
{
    db.save_transaction_info(account_id, trans);
    state.create_transaction_open = false;
    reload_wallet();
}

void Controller::delete_transaction(int transaction_id, int account_id)
{
    db.delete_transaction(transaction_id, account_id);
    reload_wallet();
}

const std::vector<Transaction_info>& Controller::get_transactions(int account_id)
{
    return db.get_transactions(account_id);
}

specific_range_of_transactions_info Controller::get_monthly_summary(int account_id, std::time_t start, std::time_t end)
{
    std::vector<Transaction_info> monthly = db.get_monthly_information(account_id, start, end);
    return db.get_specific_range_of_transactions_info(monthly);
}

void Controller::reload_wallet()
{
    state.wallet = this->db.load_accounts();
}

void Controller::create_internal_transfer(int account_id_from, int account_id_to, Transaction_info& trans)
{
    db.save_internal_transfer(account_id_from, account_id_to, trans);
    reload_wallet();
}

