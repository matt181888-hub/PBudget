#include <catch2/catch_test_macros.hpp>
#include "../src/storage.h"
#include "../src/core_logic.h"
#include "../src/helpers.h"
#include <ctime>
#include <cmath>

// Layer 3: Storage integration tests. Each test uses an in-memory DB (":memory:") so
// runs are isolated and do not touch mydata.db.

TEST_CASE("Storage opens and reports empty when no accounts exist", "[storage][schema]") {
    // Ensures the schema is created successfully and empty() correctly reflects zero accounts,
    // so we can rely on it for precondition checks in other tests.
    Storage store(":memory:");
    REQUIRE(store.empty());
}

TEST_CASE("save_account_info and load_accounts round-trip account data", "[storage][accounts]") {
    // Verifies that saving an account persists all core fields to the DB and that load_accounts
    // repopulates the in-memory list with matching data, guarding against schema/column drift.
    Storage store(":memory:");
    Account acc("Test Checking", Account_type::checking, 10000, true);
    store.save_account_info(acc);

    std::vector<Account_info> loaded = store.load_accounts();
    REQUIRE(loaded.size() == 1u);
    REQUIRE(loaded[0].account_name == "Test Checking");
    REQUIRE(loaded[0].account_type == "Checking");
    REQUIRE(loaded[0].money_amount == 10000);
    REQUIRE(loaded[0].initial_money_amount == 10000);
    REQUIRE(loaded[0].is_asset == true);
    REQUIRE(loaded[0].interest_rate == 0);
    REQUIRE(loaded[0].compounding_frequency == 0);
}

TEST_CASE("save_account_info with liability parameters persists optional fields", "[storage][accounts]") {
    // Ensures liability-specific columns (principal, term, credit_limit, etc.) are written
    // and read back correctly so the modify panel and reports see consistent data.
    Storage store(":memory:");
    Liability_parameters lp{};
    lp.principal = 100000;
    lp.liability_interest_rate = 500;
    lp.compounding_frequency = 12;
    lp.term = 360;
    lp.monthly_payment = 600;
    lp.remaining_balance = 90000;
    lp.remaining_term = 350;
    lp.credit_limit = 150000;
    lp.minimum_payment = 200;
    Account acc("My Mortgage", Account_type::mortgage, 90000, false, lp);
    store.save_account_info(acc);

    std::vector<Account_info> loaded = store.load_accounts();
    REQUIRE(loaded.size() == 1u);
    REQUIRE(loaded[0].account_name == "My Mortgage");
    REQUIRE(loaded[0].is_asset == false);
    REQUIRE(loaded[0].principal == 100000);
    REQUIRE(loaded[0].interest_rate == 500);
    REQUIRE(loaded[0].term == 360);
    REQUIRE(loaded[0].monthly_payment == 600);
    REQUIRE(loaded[0].remaining_balance == 90000);
    REQUIRE(loaded[0].credit_limit == 150000);
    REQUIRE(loaded[0].minimum_payment == 200);
}

TEST_CASE("modify_account_in_storage updates persisted account fields", "[storage][accounts]") {
    // Confirms that account edits are written to the DB and visible on the next load,
    // so the modify panel and any reload logic stay in sync with storage.
    Storage store(":memory:");
    Account acc("Original", Account_type::checking, 5000, true);
    store.save_account_info(acc);
    int account_id = acc.read_account_id_in_DB();

    store.modify_account_in_storage(account_id, "Updated Name", Account_type::savings, 7500,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    std::vector<Account_info> loaded = store.load_accounts();
    REQUIRE(loaded.size() == 1u);
    REQUIRE(loaded[0].account_name == "Updated Name");
    REQUIRE(loaded[0].account_type == "Savings");
    REQUIRE(loaded[0].money_amount == 7500);
}

TEST_CASE("delete_account removes account and cascades to transactions", "[storage][accounts]") {
    // Documents that deleting an account also removes its transactions (FOREIGN KEY CASCADE)
    // and that load_accounts no longer returns it, preventing orphaned or inconsistent state.
    Storage store(":memory:");
    Account acc("To Delete", Account_type::checking, 0, true);
    store.save_account_info(acc);
    int account_id = acc.read_account_id_in_DB();

    Transaction_info trans = create_transaction_info(
        account_id, 1000, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Pay", "", 0, 1000);
    store.save_transaction_info(account_id, trans);

    store.delete_account(account_id);

    std::vector<Account_info> loaded = store.load_accounts();
    REQUIRE(loaded.empty());
    store.load_all_transactions();
    REQUIRE(store.get_transactions(account_id).empty());
}

TEST_CASE("save_transaction_info persists transaction and updates account balance", "[storage][transactions]") {
    // Verifies that a single transaction is written to the DB, appears after load_transactions,
    // and that the account balance in the accounts table is updated to the new amount.
    Storage store(":memory:");
    Account acc("Checking", Account_type::checking, 10000, true);
    store.save_account_info(acc);
    int account_id = acc.read_account_id_in_DB();

    Transaction_info trans = create_transaction_info(
        account_id, -3000, Transaction_type::Need,
        Transaction_category_need::Housing, Transaction_category_want::Other,
        "Rent", "January", 10000, 7000);
    store.save_transaction_info(account_id, trans);

    REQUIRE(store.get_transactions(account_id).size() == 1u);
    REQUIRE(store.get_transactions(account_id)[0].transaction_amount == -3000);
    REQUIRE(store.get_transactions(account_id)[0].account_previous_amount == 10000);
    REQUIRE(store.get_transactions(account_id)[0].account_new_amount == 7000);
    REQUIRE(store.get_transactions(account_id)[0].transaction_name == "Rent");
    REQUIRE(store.get_transactions(account_id)[0].note == "January");

    std::vector<Account_info> loaded = store.load_accounts();
    REQUIRE(loaded.size() == 1u);
    REQUIRE(loaded[0].money_amount == 7000);
}

TEST_CASE("load_transactions after save reflects DB state", "[storage][transactions]") {
    // Ensures that after saving a transaction, load_transactions repopulates the cache from the DB
    // so callers that rely on get_transactions after a reload see the same data.
    Storage store(":memory:");
    Account acc("Checking", Account_type::checking, 0, true);
    store.save_account_info(acc);
    int account_id = acc.read_account_id_in_DB();

    Transaction_info trans = create_transaction_info(
        account_id, 5000, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Salary", "", 0, 5000);
    store.save_transaction_info(account_id, trans);

    store.load_transactions(account_id);
    const std::vector<Transaction_info>& list = store.get_transactions(account_id);
    REQUIRE(list.size() == 1u);
    REQUIRE(list[0].transaction_amount == 5000);
    REQUIRE(list[0].transaction_name == "Salary");
}

TEST_CASE("get_transactions returns empty for unknown account_id", "[storage][transactions]") {
    // Documents that requesting transactions for an account with no rows returns an empty list
    // rather than undefined behavior, so UI and controller code can safely assume a valid reference.
    Storage store(":memory:");
    REQUIRE(store.get_transactions(99999).empty());
}

TEST_CASE("delete_transaction removes row and recalculates account balance", "[storage][transactions]") {
    // Confirms that deleting a transaction removes it from the DB, refreshes the in-memory cache,
    // and recalculates the account balance from initial + remaining transactions so the balance stays consistent.
    Storage store(":memory:");
    Account acc("Checking", Account_type::checking, 0, true);
    store.save_account_info(acc);
    int account_id = acc.read_account_id_in_DB();

    Transaction_info t1 = create_transaction_info(
        account_id, 10000, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "In", "", 0, 10000);
    store.save_transaction_info(account_id, t1);

    Transaction_info t2 = create_transaction_info(
        account_id, -3000, Transaction_type::Need,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Out", "", 10000, 7000);
    store.save_transaction_info(account_id, t2);

    // Delete the outflow so the only remaining transaction is the +10000; balance becomes 10000.
    int id_to_delete = store.get_transactions(account_id)[1].transaction_id;
    store.delete_transaction(id_to_delete, account_id);

    const std::vector<Transaction_info>& list = store.get_transactions(account_id);
    REQUIRE(list.size() == 1u);
    std::vector<Account_info> loaded = store.load_accounts();
    REQUIRE(loaded[0].money_amount == 10000);
}

TEST_CASE("get_monthly_information returns only transactions in time window", "[storage][transactions]") {
    // Ensures the date filter (transaction_date >= start AND transaction_date < end) returns
    // only transactions in range, so monthly summaries and reports are correct.
    Storage store(":memory:");
    Account acc("Checking", Account_type::checking, 0, true);
    store.save_account_info(acc);
    int account_id = acc.read_account_id_in_DB();

    std::time_t jan_1 = 1704067200;  // 2024-01-01 00:00:00 UTC
    std::time_t feb_1 = 1706745600;  // 2024-02-01
    std::time_t mar_1 = 1709251200;  // 2024-03-01

    Transaction_info t1 = create_transaction_info(
        account_id, 1000, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Jan", "", 0, 1000);
    t1.ymd = jan_1 + 3600;
    store.save_transaction_info(account_id, t1);

    Transaction_info t2 = create_transaction_info(
        account_id, 2000, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Feb", "", 1000, 3000);
    t2.ymd = feb_1 + 3600;
    store.save_transaction_info(account_id, t2);

    std::vector<Transaction_info> jan = store.get_monthly_information(account_id, jan_1, feb_1);
    REQUIRE(jan.size() == 1u);
    REQUIRE(jan[0].transaction_name == "Jan");

    std::vector<Transaction_info> feb = store.get_monthly_information(account_id, feb_1, mar_1);
    REQUIRE(feb.size() == 1u);
    REQUIRE(feb[0].transaction_name == "Feb");

    std::vector<Transaction_info> empty = store.get_monthly_information(account_id, mar_1, mar_1 + 86400 * 31);
    REQUIRE(empty.empty());
}

TEST_CASE("get_specific_range_of_transactions_info aggregates money_in and money_out", "[storage][summary]") {
    // Verifies that the summary helper correctly sums positive amounts as money_in and
    // absolute negative amounts as money_out, and sets money_remaining when in > out.
    Storage store(":memory:");
    std::vector<Transaction_info> range;
    Transaction_info a, b, c;
    a.transaction_amount = 5000;
    b.transaction_amount = -2000;
    c.transaction_amount = -1000;
    range.push_back(a);
    range.push_back(b);
    range.push_back(c);

    specific_range_of_transactions_info info = store.get_specific_range_of_transactions_info(range);
    REQUIRE(info.money_in == 5000);
    REQUIRE(info.money_out == 3000);
    REQUIRE(info.money_remaining == 2000);
}

TEST_CASE("get_specific_range_of_transactions_info sets money_remaining to zero when out >= in", "[storage][summary]") {
    // Documents that when total outflows meet or exceed inflows, money_remaining is zero
    // so the UI never shows a negative "remaining" and logic stays consistent.
    Storage store(":memory:");
    std::vector<Transaction_info> range;
    Transaction_info a, b;
    a.transaction_amount = 1000;
    b.transaction_amount = -2500;
    range.push_back(a);
    range.push_back(b);

    specific_range_of_transactions_info info = store.get_specific_range_of_transactions_info(range);
    REQUIRE(info.money_in == 1000);
    REQUIRE(info.money_out == 2500);
    REQUIRE(info.money_remaining == 0);
}

TEST_CASE("save_internal_transfer creates two entries and updates both account balances", "[storage][transfer]") {
    // Ensures an internal transfer inserts one transaction row per account with correct signed
    // amounts and updates both account balances so the books stay balanced and both sides are visible.
    Storage store(":memory:");
    Account from_acc("From", Account_type::checking, 10000, true);
    Account to_acc("To", Account_type::checking, 2000, true);
    store.save_account_info(from_acc);
    store.save_account_info(to_acc);
    int from_id = from_acc.read_account_id_in_DB();
    int to_id = to_acc.read_account_id_in_DB();

    Transaction_info trans = create_transaction_info(
        from_id, 3000, Transaction_type::Internal_transfer,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Transfer", "", 10000, 7000);
    store.save_internal_transfer(from_id, to_id, trans);

    store.load_transactions(from_id);
    store.load_transactions(to_id);
    const std::vector<Transaction_info>& from_list = store.get_transactions(from_id);
    const std::vector<Transaction_info>& to_list = store.get_transactions(to_id);
    REQUIRE(from_list.size() == 1u);
    REQUIRE(to_list.size() == 1u);
    REQUIRE(from_list[0].transaction_amount == -3000);
    REQUIRE(to_list[0].transaction_amount == 3000);

    std::vector<Account_info> loaded = store.load_accounts();
    int from_balance = 0, to_balance = 0;
    for (const auto& a : loaded) {
        if (a.account_id == from_id) from_balance = a.money_amount;
        if (a.account_id == to_id) to_balance = a.money_amount;
    }
    REQUIRE(from_balance == 7000);
    REQUIRE(to_balance == 5000);
}
