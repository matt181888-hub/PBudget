#include <catch2/catch_test_macros.hpp>
#include "../src/app_controller.h"
#include "../src/future_app_state.h"
#include "../src/storage.h"
#include "../src/core_logic.h"
#include "../src/helpers.h"
#include <ctime>

// Layer 4: Controller integration tests. The Controller coordinates Storage and App_state;
// we use an in-memory DB and a fresh App_state so each test is isolated.

TEST_CASE("create_account persists account and refreshes state.wallet", "[controller][accounts]") {
    // Ensures the controller saves the account to storage and calls reload_wallet so the UI's
    // wallet list is updated, and that it closes the "new account" panel state.
    Storage store(":memory:");
    App_state state;
    state.new_account_open = true;
    Controller ctrl(state, store);

    Account acc("Main Checking", Account_type::checking, 15000, true);
    ctrl.create_account(acc);

    REQUIRE(state.wallet.size() == 1u);
    REQUIRE(state.wallet[0].account_name == "Main Checking");
    REQUIRE(state.wallet[0].account_type == "Checking");
    REQUIRE(state.wallet[0].money_amount == 15000);
    REQUIRE(state.new_account_open == false);
}

TEST_CASE("reload_wallet repopulates state.wallet from storage", "[controller][accounts]") {
    // Verifies that reload_wallet makes state.wallet reflect the current DB contents, so any
    // code that added accounts directly to storage (or after a reload) sees the correct list.
    Storage store(":memory:");
    App_state state;
    Controller ctrl(state, store);

    Account acc("Solo", Account_type::savings, 5000, true);
    store.save_account_info(acc);
    REQUIRE(state.wallet.empty());

    ctrl.reload_wallet();
    REQUIRE(state.wallet.size() == 1u);
    REQUIRE(state.wallet[0].account_name == "Solo");
    REQUIRE(state.wallet[0].money_amount == 5000);
}

TEST_CASE("modify_account updates storage and refreshes wallet; clears modify panel state", "[controller][accounts]") {
    // Confirms that account edits go through the controller to storage and that state.wallet
    // is refreshed and modify_account_index is cleared so the UI leaves edit mode.
    Storage store(":memory:");
    App_state state;
    state.modify_account_index = 0;
    Controller ctrl(state, store);

    Account acc("Before", Account_type::checking, 1000, true);
    ctrl.create_account(acc);
    int account_id = state.wallet[0].account_id;

    ctrl.modify_account(account_id, "After", Account_type::savings, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    REQUIRE(state.wallet.size() == 1u);
    REQUIRE(state.wallet[0].account_name == "After");
    REQUIRE(state.wallet[0].account_type == "Savings");
    REQUIRE(state.wallet[0].money_amount == 2000);
    REQUIRE(state.modify_account_index == -1);
}

TEST_CASE("delete_account removes account from storage and clears selection state", "[controller][accounts]") {
    // Ensures deleting an account via the controller removes it from the DB and clears
    // selected_account_index and modify_account_index so the UI does not reference a stale index.
    Storage store(":memory:");
    App_state state;
    state.selected_account_index = 0;
    state.modify_account_index = 0;
    Controller ctrl(state, store);

    Account acc("Gone", Account_type::checking, 0, true);
    ctrl.create_account(acc);
    int account_id = state.wallet[0].account_id;

    ctrl.delete_account(account_id);

    REQUIRE(state.wallet.empty());
    REQUIRE(state.selected_account_index == -1);
    REQUIRE(state.modify_account_index == -1);
}

TEST_CASE("create_transaction persists transaction and closes create-transaction panel", "[controller][transactions]") {
    // Verifies that creating a transaction through the controller saves it to storage and
    // sets create_transaction_open to false so the panel closes and the list can be refreshed.
    Storage store(":memory:");
    App_state state;
    state.create_transaction_open = true;
    Controller ctrl(state, store);

    Account acc("Checking", Account_type::checking, 10000, true);
    ctrl.create_account(acc);
    int account_id = state.wallet[0].account_id;

    Transaction_info trans = create_transaction_info(
        account_id, -2500, Transaction_type::Need,
        Transaction_category_need::Food, Transaction_category_want::Other,
        "Groceries", "", 10000, 7500);
    ctrl.create_transaction(account_id, trans);

    const std::vector<Transaction_info>& list = ctrl.get_transactions(account_id);
    REQUIRE(list.size() == 1u);
    REQUIRE(list[0].transaction_amount == -2500);
    REQUIRE(list[0].transaction_name == "Groceries");
    REQUIRE(state.create_transaction_open == false);
    REQUIRE(state.wallet[0].money_amount == 7500);
}

TEST_CASE("delete_transaction removes transaction and refreshes wallet", "[controller][transactions]") {
    // Ensures the controller deletes the transaction from storage and calls reload_wallet so
    // the account balance in state.wallet reflects the recalculated value after deletion.
    Storage store(":memory:");
    App_state state;
    Controller ctrl(state, store);

    Account acc("Checking", Account_type::checking, 0, true);
    ctrl.create_account(acc);
    int account_id = state.wallet[0].account_id;

    Transaction_info t1 = create_transaction_info(
        account_id, 10000, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "In", "", 0, 10000);
    ctrl.create_transaction(account_id, t1);

    Transaction_info t2 = create_transaction_info(
        account_id, -3000, Transaction_type::Need,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Out", "", 10000, 7000);
    ctrl.create_transaction(account_id, t2);

    // Delete the first transaction (the +10000 deposit); only the -3000 outflow remains.
    int id_to_delete = ctrl.get_transactions(account_id)[0].transaction_id;
    ctrl.delete_transaction(id_to_delete, account_id);

    REQUIRE(ctrl.get_transactions(account_id).size() == 1u);
    ctrl.reload_wallet();
    REQUIRE(state.wallet[0].money_amount == -3000);
}

TEST_CASE("get_transactions returns same data as storage for account", "[controller][read]") {
    // Documents that the controller's get_transactions is a pass-through to storage, so
    // any code using the controller to read transactions sees the same list as storage.
    Storage store(":memory:");
    App_state state;
    Controller ctrl(state, store);

    Account acc("Checking", Account_type::checking, 0, true);
    ctrl.create_account(acc);
    int account_id = state.wallet[0].account_id;

    Transaction_info trans = create_transaction_info(
        account_id, 500, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Test", "note", 0, 500);
    ctrl.create_transaction(account_id, trans);

    const std::vector<Transaction_info>& via_ctrl = ctrl.get_transactions(account_id);
    store.load_transactions(account_id);
    const std::vector<Transaction_info>& via_storage = store.get_transactions(account_id);
    REQUIRE(via_ctrl.size() == via_storage.size());
    REQUIRE(via_ctrl[0].transaction_id == via_storage[0].transaction_id);
    REQUIRE(via_ctrl[0].transaction_amount == via_storage[0].transaction_amount);
}

TEST_CASE("get_monthly_summary aggregates transactions in date range", "[controller][read]") {
    // Verifies that get_monthly_summary uses storage's monthly window and aggregation so
    // the UI or reports get correct money_in, money_out, and money_remaining for the period.
    Storage store(":memory:");
    App_state state;
    Controller ctrl(state, store);

    Account acc("Checking", Account_type::checking, 0, true);
    ctrl.create_account(acc);
    int account_id = state.wallet[0].account_id;

    std::time_t jan_1 = 1704067200;
    std::time_t feb_1 = 1706745600;
    std::time_t mar_1 = 1709251200;

    Transaction_info t1 = create_transaction_info(
        account_id, 5000, Transaction_type::Income,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Jan in", "", 0, 5000);
    t1.ymd = jan_1 + 3600;
    store.save_transaction_info(account_id, t1);

    Transaction_info t2 = create_transaction_info(
        account_id, -1000, Transaction_type::Need,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Jan out", "", 5000, 4000);
    t2.ymd = jan_1 + 7200;
    store.save_transaction_info(account_id, t2);

    specific_range_of_transactions_info summary = ctrl.get_monthly_summary(account_id, jan_1, feb_1);
    REQUIRE(summary.money_in == 5000);
    REQUIRE(summary.money_out == 1000);
    REQUIRE(summary.money_remaining == 4000);
}

TEST_CASE("create_internal_transfer updates both accounts and refreshes wallet", "[controller][transfer]") {
    // Ensures an internal transfer through the controller creates both transaction rows and
    // that reload_wallet runs so state.wallet shows updated balances for both accounts.
    Storage store(":memory:");
    App_state state;
    Controller ctrl(state, store);

    Account from("From", Account_type::checking, 8000, true);
    Account to("To", Account_type::checking, 2000, true);
    ctrl.create_account(from);
    ctrl.create_account(to);
    int from_id = state.wallet[0].account_id;
    int to_id = state.wallet[1].account_id;

    Transaction_info trans = create_transaction_info(
        from_id, 2000, Transaction_type::Internal_transfer,
        Transaction_category_need::Other, Transaction_category_want::Other,
        "Move", "", 8000, 6000);
    ctrl.create_internal_transfer(from_id, to_id, trans);

    REQUIRE(ctrl.get_transactions(from_id).size() == 1u);
    REQUIRE(ctrl.get_transactions(to_id).size() == 1u);
    REQUIRE(ctrl.get_transactions(from_id)[0].transaction_amount == -2000);
    REQUIRE(ctrl.get_transactions(to_id)[0].transaction_amount == 2000);

    REQUIRE(state.wallet.size() == 2u);
    int from_balance = 0, to_balance = 0;
    for (const auto& a : state.wallet) {
        if (a.account_id == from_id) from_balance = a.money_amount;
        if (a.account_id == to_id) to_balance = a.money_amount;
    }
    REQUIRE(from_balance == 6000);
    REQUIRE(to_balance == 4000);
}

TEST_CASE("get_transactions returns empty for unknown account_id", "[controller][read]") {
    // Documents that the controller returns an empty list for an account with no transactions
    // (or invalid id) so callers can safely iterate without null checks.
    Storage store(":memory:");
    App_state state;
    Controller ctrl(state, store);

    REQUIRE(ctrl.get_transactions(99999).empty());
}
