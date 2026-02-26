extern "C"{
    #include "../external/sqlite/sqlite3.h"
}
#include <iostream>
#include <cstring>
#include "core_logic.h"
#include "helpers.h"
#include "storage.h"


//create the storage object if it doesnt exist yet, and open the database
Storage::Storage()           
    {

        int rc;

        rc = sqlite3_open("mydata.db", &db);

        if (rc) 
        {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        }
        else 
        {
            std::cout << "Opened database successfully" << std::endl;
        }

        const char* sql_accounts = 
        R"(CREATE TABLE IF NOT EXISTS accounts(
            id INTEGER PRIMARY KEY, 
            money_amount INTEGER, 
            account_name TEXT, 
            account_type TEXT,
            initial_money_amount INTEGER DEFAULT 0,
            is_asset INTEGER DEFAULT 1,
            interest_rate INTEGER DEFAULT 0,
            compounding_frequency INTEGER DEFAULT 0,
            principal INTEGER DEFAULT 0,
            term INTEGER DEFAULT 0,
            monthly_payment INTEGER DEFAULT 0,
            remaining_balance INTEGER DEFAULT 0,
            remaining_term INTEGER DEFAULT 0,
            remaining_interest INTEGER DEFAULT 0,
            remaining_principal INTEGER DEFAULT 0,
            remaining_total INTEGER DEFAULT 0,
            credit_limit INTEGER DEFAULT 0,
            minimum_payment INTEGER DEFAULT 0
        );)";

        rc = sqlite3_exec(db, sql_accounts, NULL, 0, NULL);

        if (rc != SQLITE_OK) 
        {
            std::cerr << "SQL error creating accounts table" << std::endl;
        } 
        else 
        {
            std::cout << "Accounts table created successfully" << std::endl;
        }

        // Create transactions table
        const char* sql_transactions = 
        R"(CREATE TABLE IF NOT EXISTS transactions_table(
            id INTEGER PRIMARY KEY,
            account_id INTEGER, 
            transaction_amount INTEGER,
            transaction_type TEXT,
            previous_amount INTEGER,
            new_amount INTEGER,
            transaction_date INTEGER,
            transaction_name TEXT,
            note TEXT,
            transaction_category TEXT,
            FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE
        );)";

        rc = sqlite3_exec(db, sql_transactions, NULL, 0, NULL);

        if (rc != SQLITE_OK) 
        {
            std::cerr << "SQL error creating transactions table" << std::endl;
        } 
        else 
        {
            std::cout << "Transactions table created successfully" << std::endl;
        }
    }

//close the database when the storage object is destroyed
Storage::~Storage()
{
    if(db)
    {
        sqlite3_close(db);
    }
    db = nullptr;
}

//save the account info to the database
void Storage::save_account_info(Account &acc)
{
    const char* sql_account_type;
    int sql_account_money;
    int rc;

    std::string name_str = acc.read_account_name();
    const char *sql_account_name = name_str.c_str();
    
    sql_account_money = acc.read_money();

    sql_account_type = account_type_to_string(acc.read_account_type());

    const char* tail;
    const char* instructions = 
    R"(INSERT INTO accounts(money_amount, account_name, account_type, initial_money_amount, is_asset, interest_rate, compounding_frequency, principal, term, monthly_payment, remaining_balance, remaining_term, remaining_interest, remaining_principal, remaining_total, credit_limit, minimum_payment)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db, instructions, -1, &stmt, &tail);

    sqlite3_bind_int(stmt, 1, sql_account_money);
    sqlite3_bind_text(stmt, 2, sql_account_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sql_account_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, acc.read_initial_money());
    sqlite3_bind_int(stmt, 5, acc.is_asset() ? 1 : 0);
    if (acc.is_asset())
    {
        sqlite3_bind_int(stmt, 6, acc.read_asset_interest_rate());
        sqlite3_bind_int(stmt, 7, acc.read_asset_compounding_frequency());
        sqlite3_bind_int(stmt, 8, 0);
        sqlite3_bind_int(stmt, 9, 0);
        sqlite3_bind_int(stmt, 10, 0);
        sqlite3_bind_int(stmt, 11, 0);
        sqlite3_bind_int(stmt, 12, 0);
        sqlite3_bind_int(stmt, 13, 0);
        sqlite3_bind_int(stmt, 14, 0);
        sqlite3_bind_int(stmt, 15, 0);
        sqlite3_bind_int(stmt, 16, 0);
        sqlite3_bind_int(stmt, 17, 0);
    }
    else
    {
        sqlite3_bind_int(stmt, 6, acc.read_liability_interest_rate());
        sqlite3_bind_int(stmt, 7, acc.read_liability_compounding_frequency());
        sqlite3_bind_int(stmt, 8, acc.read_liability_principal());
        sqlite3_bind_int(stmt, 9, acc.read_term());
        sqlite3_bind_int(stmt, 10, acc.read_monthly_payment());
        sqlite3_bind_int(stmt, 11, acc.read_remaining_balance());
        sqlite3_bind_int(stmt, 12, acc.read_remaining_term());
        sqlite3_bind_int(stmt, 13, acc.read_remaining_interest());
        sqlite3_bind_int(stmt, 14, acc.read_remaining_principal());
        sqlite3_bind_int(stmt, 15, acc.read_remaining_total());
        sqlite3_bind_int(stmt, 16, acc.read_credit_limit());
        sqlite3_bind_int(stmt, 17, acc.read_minimum_payment());
    }
    sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    //create the account info object
    Account_info acc_info;
    acc_info.account_id = sqlite3_last_insert_rowid(db);
    acc_info.money_amount = sql_account_money;
    acc_info.initial_money_amount = acc.read_initial_money();
    acc_info.account_name = name_str;
    acc_info.account_type = sql_account_type;
    acc_info.is_asset = acc.is_asset();
    acc_info.interest_rate = acc.is_asset() ? acc.read_asset_interest_rate() : acc.read_liability_interest_rate();
    acc_info.compounding_frequency = acc.is_asset() ? acc.read_asset_compounding_frequency() : acc.read_liability_compounding_frequency();
    acc_info.principal = acc.read_liability_principal();
    acc_info.term = acc.read_term();
    acc_info.monthly_payment = acc.read_monthly_payment();
    acc_info.remaining_balance = acc.read_remaining_balance();
    acc_info.remaining_term = acc.read_remaining_term();
    acc_info.remaining_interest = acc.read_remaining_interest();
    acc_info.remaining_principal = acc.read_remaining_principal();
    acc_info.remaining_total = acc.read_remaining_total();
    acc_info.credit_limit = acc.read_credit_limit();
    acc_info.minimum_payment = acc.read_minimum_payment();
    //set Account's id 
    acc.set_account_id(acc_info.account_id);
    accounts_vec.push_back(acc_info);
};

//check if the database is empty
bool Storage::empty()
{
    sqlite3_stmt* stmt;
    const char* instructions = "SELECT COUNT(*) FROM accounts;";
    int count = 0;

    if (sqlite3_prepare_v2(db, instructions, -1, &stmt, NULL) == SQLITE_OK) 
    {
        if (sqlite3_step(stmt) == SQLITE_ROW) 
        {
            count = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt); 

    return (count == 0);
}

std::vector<Account_info> Storage::load_accounts()
{
    accounts_vec.clear();

    sqlite3_stmt* stmt;
    const char* instructions = "SELECT * FROM accounts;";
    int rc;

    rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Account_info acc_info;
        acc_info.account_id = sqlite3_column_int(stmt, 0);
        acc_info.money_amount = sqlite3_column_int(stmt, 1);
        acc_info.account_name = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        acc_info.account_type = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        acc_info.initial_money_amount = (sqlite3_column_count(stmt) > 4) ? sqlite3_column_int(stmt, 4) : 0;
        acc_info.is_asset = (sqlite3_column_count(stmt) > 5) ? sqlite3_column_int(stmt, 5) : 0;
        acc_info.interest_rate = (sqlite3_column_count(stmt) > 6) ? sqlite3_column_int(stmt, 6) : 0;
        acc_info.compounding_frequency = (sqlite3_column_count(stmt) > 7) ? sqlite3_column_int(stmt, 7) : 0;
        acc_info.principal = (sqlite3_column_count(stmt) > 8) ? sqlite3_column_int(stmt, 8) : 0;
        acc_info.term = (sqlite3_column_count(stmt) > 9) ? sqlite3_column_int(stmt, 9) : 0;
        acc_info.monthly_payment = (sqlite3_column_count(stmt) > 10) ? sqlite3_column_int(stmt, 10) : 0;
        acc_info.remaining_balance = (sqlite3_column_count(stmt) > 11) ? sqlite3_column_int(stmt, 11) : 0;
        acc_info.remaining_term = (sqlite3_column_count(stmt) > 12) ? sqlite3_column_int(stmt, 12) : 0;
        acc_info.remaining_interest = (sqlite3_column_count(stmt) > 13) ? sqlite3_column_int(stmt, 13) : 0;
        acc_info.remaining_principal = (sqlite3_column_count(stmt) > 14) ? sqlite3_column_int(stmt, 14) : 0;
        acc_info.remaining_total = (sqlite3_column_count(stmt) > 15) ? sqlite3_column_int(stmt, 15) : 0;
        acc_info.credit_limit = (sqlite3_column_count(stmt) > 16) ? sqlite3_column_int(stmt, 16) : 0;
        acc_info.minimum_payment = (sqlite3_column_count(stmt) > 17) ? sqlite3_column_int(stmt, 17) : 0;
        accounts_vec.push_back(acc_info);
    }

    sqlite3_finalize(stmt);

    return accounts_vec;
}

void Storage::save_transaction_info(int account_id, Transaction_info &trans)
{
    if (!db) {
        std::cerr << "save_transaction_info: database not open" << std::endl;
        return;
    }

    const char* sql_transaction_type = transaction_type_to_string(trans.type_of_transaction);

    const char* sql_transaction_category = nullptr;
    
    if (trans.type_of_transaction == Transaction_type::Need) 
    {
        sql_transaction_category = transaction_category_need_to_string(trans.transaction_category_need);
    } 
    else if (trans.type_of_transaction == Transaction_type::Want) 
    {
        sql_transaction_category = transaction_category_want_to_string(trans.transaction_category_want);
    }

    const char* tail;
    const char* instructions =
    R"(INSERT INTO transactions_table(account_id, transaction_amount, transaction_type, previous_amount, new_amount, transaction_date, transaction_name, note, transaction_category)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_stmt* update_stmt = nullptr;

    auto rollback_transaction = [this]() {
        char* rollback_err = nullptr;
        int rollback_rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &rollback_err);
        if (rollback_rc != SQLITE_OK) {
            std::cerr << "save_transaction_info ROLLBACK failed: "
                      << (rollback_err ? rollback_err : sqlite3_errmsg(db)) << std::endl;
            sqlite3_free(rollback_err);
        }
    };

    char* begin_err = nullptr;
    int rc = sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, &begin_err);
    if (rc != SQLITE_OK) {
        std::cerr << "save_transaction_info BEGIN failed: "
                  << (begin_err ? begin_err : sqlite3_errmsg(db)) << std::endl;
        sqlite3_free(begin_err);
        return;
    }

    rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, &tail);
    if (rc != SQLITE_OK) {
        std::cerr << "save_transaction_info prepare failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }

    sqlite3_bind_int(stmt, 1, account_id);
    sqlite3_bind_int(stmt, 2, trans.transaction_amount);
    sqlite3_bind_text(stmt, 3, sql_transaction_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, trans.account_previous_amount);
    sqlite3_bind_int(stmt, 5, trans.account_new_amount);
    sqlite3_bind_int(stmt, 6, static_cast<int>(trans.ymd));
    sqlite3_bind_text(stmt, 7, trans.transaction_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, trans.note.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, sql_transaction_category, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        std::cerr << "save_transaction_info INSERT failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    trans.transaction_id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);

    // Update the account balance in the accounts table
    const char* update_sql = "UPDATE accounts SET money_amount = ? WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "save_transaction_info UPDATE prepare failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    sqlite3_bind_int(update_stmt, 1, trans.account_new_amount);
    sqlite3_bind_int(update_stmt, 2, account_id);
    rc = sqlite3_step(update_stmt);
    sqlite3_finalize(update_stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "save_transaction_info UPDATE failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }

    char* commit_err = nullptr;
    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &commit_err);
    if (rc != SQLITE_OK) {
        std::cerr << "save_transaction_info COMMIT failed: "
                  << (commit_err ? commit_err : sqlite3_errmsg(db)) << std::endl;
        sqlite3_free(commit_err);
        rollback_transaction();
        return;
    }

    // Keep in-memory cache in sync only after both DB writes commit.
    transactions_by_account[account_id].push_back(trans);
}
void Storage::save_internal_transfer(int account_id_from, int account_id_to, Transaction_info &trans)
{
    if (!db) {
        std::cerr << "save_internal_transfer: database not open" << std::endl;
        return;
    }

    const char* sql_transaction_type = transaction_type_to_string(trans.type_of_transaction);

    auto rollback_transaction = [this]() {
        char* rollback_err = nullptr;
        int rollback_rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &rollback_err);
        if (rollback_rc != SQLITE_OK) {
            std::cerr << "save_internal_transfer ROLLBACK failed: "
                      << (rollback_err ? rollback_err : sqlite3_errmsg(db)) << std::endl;
            sqlite3_free(rollback_err);
        }
    };

    char* begin_err = nullptr;
    int rc = sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, &begin_err);
    if (rc != SQLITE_OK) {
        std::cerr << "save_internal_transfer BEGIN failed: "
                  << (begin_err ? begin_err : sqlite3_errmsg(db)) << std::endl;
        sqlite3_free(begin_err);
        return;
    }

    // Read current balance and is_asset for an account within this transaction
    auto read_account = [this](int account_id, int &balance, bool &is_asset) -> bool {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT money_amount, is_asset FROM accounts WHERE id = ?;";
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) return false;
        sqlite3_bind_int(stmt, 1, account_id);
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return false;
        }
        balance = sqlite3_column_int(stmt, 0);
        is_asset = sqlite3_column_int(stmt, 1) != 0;
        sqlite3_finalize(stmt);
        return true;
    };

    int from_balance = 0, to_balance = 0;
    bool from_is_asset = true, to_is_asset = true;

    if (!read_account(account_id_from, from_balance, from_is_asset) ||
        !read_account(account_id_to, to_balance, to_is_asset)) {
        std::cerr << "save_internal_transfer: failed to read account info" << std::endl;
        rollback_transaction();
        return;
    }

    int transfer_amount = std::abs(trans.transaction_amount);

    int new_from_balance = balance_after_transaction(from_balance, transfer_amount, false, from_is_asset);
    int new_to_balance   = balance_after_transaction(to_balance,   transfer_amount, true,  to_is_asset);

    int from_delta = new_from_balance - from_balance;
    int to_delta   = new_to_balance   - to_balance;

    // Insert transaction row for source account
    const char* insert_sql =
    R"(INSERT INTO transactions_table(account_id, transaction_amount, transaction_type, previous_amount, new_amount, transaction_date, transaction_name, note, transaction_category)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "save_internal_transfer INSERT (from) prepare failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    sqlite3_bind_int(stmt, 1, account_id_from);
    sqlite3_bind_int(stmt, 2, from_delta);
    sqlite3_bind_text(stmt, 3, sql_transaction_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, from_balance);
    sqlite3_bind_int(stmt, 5, new_from_balance);
    sqlite3_bind_int(stmt, 6, static_cast<int>(trans.ymd));
    sqlite3_bind_text(stmt, 7, trans.transaction_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, trans.note.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_null(stmt, 9);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        std::cerr << "save_internal_transfer INSERT (from) failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    int from_transaction_id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);

    // Insert transaction row for destination account
    stmt = nullptr;
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "save_internal_transfer INSERT (to) prepare failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    sqlite3_bind_int(stmt, 1, account_id_to);
    sqlite3_bind_int(stmt, 2, to_delta);
    sqlite3_bind_text(stmt, 3, sql_transaction_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, to_balance);
    sqlite3_bind_int(stmt, 5, new_to_balance);
    sqlite3_bind_int(stmt, 6, static_cast<int>(trans.ymd));
    sqlite3_bind_text(stmt, 7, trans.transaction_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, trans.note.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_null(stmt, 9);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        std::cerr << "save_internal_transfer INSERT (to) failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    int to_transaction_id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);

    // Update source account balance
    const char* update_sql = "UPDATE accounts SET money_amount = ? WHERE id = ?;";
    sqlite3_stmt* update_stmt = nullptr;
    rc = sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "save_internal_transfer UPDATE (from) prepare failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    sqlite3_bind_int(update_stmt, 1, new_from_balance);
    sqlite3_bind_int(update_stmt, 2, account_id_from);
    rc = sqlite3_step(update_stmt);
    sqlite3_finalize(update_stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "save_internal_transfer UPDATE (from) failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }

    // Update destination account balance
    update_stmt = nullptr;
    rc = sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "save_internal_transfer UPDATE (to) prepare failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }
    sqlite3_bind_int(update_stmt, 1, new_to_balance);
    sqlite3_bind_int(update_stmt, 2, account_id_to);
    rc = sqlite3_step(update_stmt);
    sqlite3_finalize(update_stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "save_internal_transfer UPDATE (to) failed: " << sqlite3_errmsg(db) << std::endl;
        rollback_transaction();
        return;
    }

    char* commit_err = nullptr;
    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &commit_err);
    if (rc != SQLITE_OK) {
        std::cerr << "save_internal_transfer COMMIT failed: "
                  << (commit_err ? commit_err : sqlite3_errmsg(db)) << std::endl;
        sqlite3_free(commit_err);
        rollback_transaction();
        return;
    }

    // Update in-memory cache for both accounts after successful commit
    Transaction_info from_trans;
    from_trans.transaction_id = from_transaction_id;
    from_trans.account_id = account_id_from;
    from_trans.transaction_amount = from_delta;
    from_trans.type_of_transaction = Transaction_type::Internal_transfer;
    from_trans.transaction_category_need = Transaction_category_need::Other;
    from_trans.transaction_category_want = Transaction_category_want::Other;
    from_trans.account_previous_amount = from_balance;
    from_trans.account_new_amount = new_from_balance;
    from_trans.ymd = trans.ymd;
    from_trans.transaction_name = trans.transaction_name;
    from_trans.note = trans.note;
    transactions_by_account[account_id_from].push_back(from_trans);

    Transaction_info to_trans;
    to_trans.transaction_id = to_transaction_id;
    to_trans.account_id = account_id_to;
    to_trans.transaction_amount = to_delta;
    to_trans.type_of_transaction = Transaction_type::Internal_transfer;
    to_trans.transaction_category_need = Transaction_category_need::Other;
    to_trans.transaction_category_want = Transaction_category_want::Other;
    to_trans.account_previous_amount = to_balance;
    to_trans.account_new_amount = new_to_balance;
    to_trans.ymd = trans.ymd;
    to_trans.transaction_name = trans.transaction_name;
    to_trans.note = trans.note;
    transactions_by_account[account_id_to].push_back(to_trans);
}

void Storage::load_transactions(int account_id)
{
    transactions_by_account[account_id].clear();

    sqlite3_stmt* stmt = nullptr;
    const char* instructions = "SELECT * FROM transactions_table WHERE account_id = ?;";
    int rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "load_transactions prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    sqlite3_bind_int(stmt, 1, account_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Transaction_info trans_info = get_transaction_info_from_stmt(stmt);
        transactions_by_account[account_id].push_back(trans_info);
    }
    sqlite3_finalize(stmt);
}

void Storage::load_all_transactions()
{
    transactions_by_account.clear();

    sqlite3_stmt* stmt = nullptr;
    const char* instructions = "SELECT * FROM transactions_table ORDER BY account_id, id;";
    int rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "load_all_transactions prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Transaction_info trans_info = get_transaction_info_from_stmt(stmt);
        transactions_by_account[trans_info.account_id].push_back(trans_info);
    }
    sqlite3_finalize(stmt);
}

const std::vector<Transaction_info>& Storage::get_transactions(int account_id)
{
    static const std::vector<Transaction_info> empty;
    auto it = transactions_by_account.find(account_id);
    if (it == transactions_by_account.end())
        return empty;
    return it->second;
}

void Storage::delete_transaction(int transaction_id, int account_id)
{
    sqlite3_stmt* stmt = nullptr;
    const char* instructions = "DELETE FROM transactions_table WHERE id = ?;";
    int rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "delete_transaction prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    sqlite3_bind_int(stmt, 1, transaction_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "delete_transaction failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Get initial balance for this account (used when all transactions are deleted)
    int initial_balance = 0;
    sqlite3_stmt* init_stmt = nullptr;
    const char* init_sql = "SELECT COALESCE(initial_money_amount, 0) FROM accounts WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, init_sql, -1, &init_stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(init_stmt, 1, account_id);
        if (sqlite3_step(init_stmt) == SQLITE_ROW)
            initial_balance = sqlite3_column_int(init_stmt, 0);
        sqlite3_finalize(init_stmt);
    }

    // Recalculate the account balance: initial + sum of remaining transactions
    const char* sum_sql = "SELECT COALESCE(SUM(transaction_amount), 0) FROM transactions_table WHERE account_id = ?;";
    sqlite3_stmt* sum_stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sum_sql, -1, &sum_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "delete_transaction SUM prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    sqlite3_bind_int(sum_stmt, 1, account_id);
    int transaction_sum = 0;
    if (sqlite3_step(sum_stmt) == SQLITE_ROW) {
        transaction_sum = sqlite3_column_int(sum_stmt, 0);
    }
    sqlite3_finalize(sum_stmt);
    int new_balance = initial_balance + transaction_sum;

    // Update the account's balance in the accounts table
    const char* update_sql = "UPDATE accounts SET money_amount = ? WHERE id = ?;";
    sqlite3_stmt* update_stmt = nullptr;
    rc = sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "delete_transaction UPDATE prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    sqlite3_bind_int(update_stmt, 1, new_balance);
    sqlite3_bind_int(update_stmt, 2, account_id);
    rc = sqlite3_step(update_stmt);
    sqlite3_finalize(update_stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "delete_transaction UPDATE failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Refresh in-memory transactions cache for this account
    load_transactions(account_id);
}

std::vector<Transaction_info> Storage::get_monthly_information(int account_id, std::time_t start_time, std::time_t end_time)
{
    std::vector<Transaction_info> monthly_transactions;
    sqlite3_stmt* stmt = nullptr;
    const char* instructions = "SELECT * FROM transactions_table WHERE account_id = ? AND transaction_date >= ? AND transaction_date < ?;";
    int rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "load_all_transactions prepare failed: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_bind_int(stmt, 1, account_id);
    sqlite3_bind_int(stmt, 2, start_time);
    sqlite3_bind_int(stmt, 3, end_time);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Transaction_info trans_info = get_transaction_info_from_stmt(stmt);
        monthly_transactions.push_back(trans_info);
    }
    sqlite3_finalize(stmt);
    return monthly_transactions;
}

struct specific_range_of_transactions_info Storage::get_specific_range_of_transactions_info(std::vector<Transaction_info> &range_of_transactions)
{
    specific_range_of_transactions_info range_of_transactions_info;
    for (const Transaction_info &trans : range_of_transactions)
    {
        if (trans.transaction_amount > 0)
        {
            range_of_transactions_info.money_in += trans.transaction_amount;
        }
        else
        {
            range_of_transactions_info.money_out += std::abs(trans.transaction_amount);
        }
    }
    if (range_of_transactions_info.money_in > range_of_transactions_info.money_out)
    {
        range_of_transactions_info.money_remaining = range_of_transactions_info.money_in - range_of_transactions_info.money_out;
    }
    else
    {
        range_of_transactions_info.money_remaining = 0;
    }
    return range_of_transactions_info;
}

void Storage::modify_account_in_storage(int account_id, std::string new_account_name, Account_type new_type_of_account, int new_money,
                                        int interest_rate, int compounding_frequency, int principal, int term, int monthly_payment, 
                                        int remaining_balance, int remaining_term, int remaining_interest, int remaining_principal, 
                                        int remaining_total, int credit_limit, int minimum_payment)
{
    sqlite3_stmt* stmt = nullptr;
    const char* sql_account_type;
    const char* instructions = "UPDATE accounts SET account_name = ?, account_type = ?, money_amount = ?, is_asset = ?, interest_rate = ?, compounding_frequency = ?, principal = ?, term = ?, monthly_payment = ?, remaining_balance = ?, remaining_term = ?, remaining_interest = ?, remaining_principal = ?, remaining_total = ?, credit_limit = ?, minimum_payment = ? WHERE id = ?;";
    int rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "modify_account_in_storage prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sql_account_type = account_type_to_string(new_type_of_account);

    sqlite3_bind_text(stmt, 1, new_account_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sql_account_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, new_money);
    if(new_type_of_account == Account_type::checking || new_type_of_account == Account_type::savings || new_type_of_account == Account_type::investments)
    {
        sqlite3_bind_int(stmt, 4, 1);
        sqlite3_bind_int(stmt, 5, interest_rate);
        sqlite3_bind_int(stmt, 6, compounding_frequency);
        sqlite3_bind_int(stmt, 7, principal);
        sqlite3_bind_int(stmt, 8, term);
        sqlite3_bind_int(stmt, 9, monthly_payment);
        sqlite3_bind_int(stmt, 10, remaining_balance);
        sqlite3_bind_int(stmt, 11, remaining_term);
        sqlite3_bind_int(stmt, 12, remaining_interest);
        sqlite3_bind_int(stmt, 13, remaining_principal);
        sqlite3_bind_int(stmt, 14, remaining_total);
        sqlite3_bind_int(stmt, 15, credit_limit);
        sqlite3_bind_int(stmt, 16, minimum_payment);
    }
    else 
    {
        sqlite3_bind_int(stmt, 4, 0);
        sqlite3_bind_int(stmt, 5, interest_rate);
        sqlite3_bind_int(stmt, 6, compounding_frequency);
        sqlite3_bind_int(stmt, 7, principal);
        sqlite3_bind_int(stmt, 8, term);
        sqlite3_bind_int(stmt, 9, monthly_payment);
        sqlite3_bind_int(stmt, 10, remaining_balance);
        sqlite3_bind_int(stmt, 11, remaining_term);
        sqlite3_bind_int(stmt, 12, remaining_interest);
        sqlite3_bind_int(stmt, 13, remaining_principal);
        sqlite3_bind_int(stmt, 14, remaining_total);
        sqlite3_bind_int(stmt, 15, credit_limit);
        sqlite3_bind_int(stmt, 16, minimum_payment);
    }
    sqlite3_bind_int(stmt, 17, account_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "modify_account_in_storage failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    // myDB.load_accounts(); will refresh the accounts_vec
    // myDB.load_all_transactions(); will refresh the transactions_by_account map
}

void Storage::delete_account(int account_id)
{
    sqlite3_stmt* stmt = nullptr;
    const char* instructions = "DELETE FROM accounts WHERE id = ?;";
    int rc = sqlite3_prepare_v2(db, instructions, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "delete_account prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    sqlite3_bind_int(stmt, 1, account_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "delete_account failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    sqlite3_stmt* delete_transactions_stmt = nullptr;
    const char* delete_transactions_sql = "DELETE FROM transactions_table WHERE account_id = ?;";
    rc = sqlite3_prepare_v2(db, delete_transactions_sql, -1, &delete_transactions_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "delete_account delete_transactions prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    sqlite3_bind_int(delete_transactions_stmt, 1, account_id);
    rc = sqlite3_step(delete_transactions_stmt);
    sqlite3_finalize(delete_transactions_stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "delete_account delete_transactions failed: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    // myDB.load_accounts(); will refresh the accounts_vec
    // myDB.load_all_transactions(); will refresh the transactions_by_account map
}
Transaction_info Storage::get_transaction_info_from_stmt(sqlite3_stmt* stmt)
{
    Transaction_info trans_info;
    trans_info.transaction_id = sqlite3_column_int(stmt, 0);
    trans_info.account_id = sqlite3_column_int(stmt, 1);
    trans_info.transaction_amount = sqlite3_column_int(stmt, 2);
    trans_info.type_of_transaction = transaction_type_from_string(
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));

    trans_info.transaction_category_need = Transaction_category_need::Other;
    trans_info.transaction_category_want = Transaction_category_want::Other;
    if (sqlite3_column_count(stmt) >= 10) {
        const char* cat_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        if (cat_text) {
            if (trans_info.type_of_transaction == Transaction_type::Need)
                trans_info.transaction_category_need = transaction_category_need_from_string(cat_text);
            else if (trans_info.type_of_transaction == Transaction_type::Want)
                trans_info.transaction_category_want = transaction_category_want_from_string(cat_text);
        }
    }

    trans_info.account_previous_amount = sqlite3_column_int(stmt, 4);
    trans_info.account_new_amount = sqlite3_column_int(stmt, 5);
    trans_info.ymd = static_cast<std::time_t>(sqlite3_column_int(stmt, 6));

    const char* name_p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    trans_info.transaction_name = name_p ? name_p : "";
    const char* note_p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    trans_info.note = note_p ? note_p : "";

    return trans_info;
}