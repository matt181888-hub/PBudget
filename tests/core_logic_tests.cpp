#include <catch2/catch_test_macros.hpp>
#include "../src/core_logic.h"

// Layer 1: core_logic unit tests

TEST_CASE("balance_after_transaction for asset accounts", "[core_logic][balance]") {
    SECTION("deposit increases asset balance") {
        int current_balance = 1000;
        int amount = 500;
        bool is_deposit = true;
        bool is_asset = true;

        int result = balance_after_transaction(current_balance, amount, is_deposit, is_asset);
        REQUIRE(result == 1500);
    }

    SECTION("withdrawal decreases asset balance") {
        int current_balance = 1000;
        int amount = 500;
        bool is_deposit = false;
        bool is_asset = true;

        int result = balance_after_transaction(current_balance, amount, is_deposit, is_asset);
        REQUIRE(result == 500);
    }

    SECTION("withdrawal larger than balance can produce negative balance") {
        int current_balance = 300;
        int amount = 500;
        bool is_deposit = false;
        bool is_asset = true;

        int result = balance_after_transaction(current_balance, amount, is_deposit, is_asset);
        REQUIRE(result == -200);
    }
}

TEST_CASE("balance_after_transaction for liability accounts", "[core_logic][balance]") {
    SECTION("payment (deposit) decreases liability balance") {
        int current_balance = 1000;
        int amount = 300;
        bool is_deposit = true;
        bool is_asset = false;

        int result = balance_after_transaction(current_balance, amount, is_deposit, is_asset);
        REQUIRE(result == 700);
    }

    SECTION("charge (withdrawal) increases liability balance") {
        int current_balance = 1000;
        int amount = 300;
        bool is_deposit = false;
        bool is_asset = false;

        int result = balance_after_transaction(current_balance, amount, is_deposit, is_asset);
        REQUIRE(result == 1300);
    }
}

TEST_CASE("balance_after_transaction handles negative starting balances", "[core_logic][balance][edge]") {
    SECTION("asset account starting negative can move toward zero or further negative") {
        int current_balance = -500;
        int amount = 200;

        int result_deposit = balance_after_transaction(current_balance, amount, true, true);
        int result_withdraw = balance_after_transaction(current_balance, amount, false, true);

        REQUIRE(result_deposit == -300);  // deposit reduces the overdraft
        REQUIRE(result_withdraw == -700); // withdrawal deepens the overdraft
    }

    SECTION("liability account starting negative behaves consistently with sign rule") {
        int current_balance = -1000;
        int amount = 300;

        int result_payment = balance_after_transaction(current_balance, amount, true, false);
        int result_charge = balance_after_transaction(current_balance, amount, false, false);

        REQUIRE(result_payment == -1300);
        REQUIRE(result_charge == -700);
    }
}

TEST_CASE("balance_after_transaction with zero amount is a no-op", "[core_logic][balance]") {
    int current_balance = 123456;
    int amount = 0;

    int result_deposit_asset = balance_after_transaction(current_balance, amount, true, true);
    int result_withdraw_asset = balance_after_transaction(current_balance, amount, false, true);
    int result_deposit_liability = balance_after_transaction(current_balance, amount, true, false);
    int result_withdraw_liability = balance_after_transaction(current_balance, amount, false, false);

    REQUIRE(result_deposit_asset == current_balance);
    REQUIRE(result_withdraw_asset == current_balance);
    REQUIRE(result_deposit_liability == current_balance);
    REQUIRE(result_withdraw_liability == current_balance);
}

TEST_CASE("Account constructor populates core fields and asset flag", "[core_logic][account]") {
    SECTION("basic constructor sets name, type, balances, and asset flag") {
        Account acc("Checking 1", Account_type::checking, 1000, true);

        REQUIRE(acc.read_account_name() == "Checking 1");
        REQUIRE(acc.read_account_type() == Account_type::checking);
        REQUIRE(acc.read_money() == 1000);
        REQUIRE(acc.read_initial_money() == 1000);
        REQUIRE(acc.is_asset());
    }

    SECTION("set_account_id updates database row id while default is zero") {
        Account acc("Unnamed", Account_type::checking, 0, true);
        REQUIRE(acc.read_account_id_in_DB() == 0);

        acc.set_account_id(42);
        REQUIRE(acc.read_account_id_in_DB() == 42);
    }
}

TEST_CASE("Account constructors respect explicit is_asset override", "[core_logic][account]") {
    SECTION("asset-like type can be forced to behave as liability") {
        Account acc("Weird Savings", Account_type::savings, 1000, false);
        REQUIRE(acc.read_account_type() == Account_type::savings);
        REQUIRE_FALSE(acc.is_asset());
    }

    SECTION("liability-like type can be forced to behave as asset") {
        Account acc("Special Loan", Account_type::loan, 2000, true);
        REQUIRE(acc.read_account_type() == Account_type::loan);
        REQUIRE(acc.is_asset());
    }
}

TEST_CASE("Account name and modify_account update mutable fields only", "[core_logic][account]") {
    Account acc("Initial Name", Account_type::checking, 1000, true);

    SECTION("constructor sets name, type, money, initial balance") {
        REQUIRE(acc.read_account_name() == "Initial Name");
        REQUIRE(acc.read_account_type() == Account_type::checking);
        REQUIRE(acc.read_money() == 1000);
        REQUIRE(acc.read_initial_money() == 1000);
    }

    SECTION("modify_account changes name, type, and current money but not initial money") {
        acc.modify_account("Updated Name", Account_type::savings, 1500);
        REQUIRE(acc.read_account_name() == "Updated Name");
        REQUIRE(acc.read_account_type() == Account_type::savings);
        REQUIRE(acc.read_money() == 1500);
        REQUIRE(acc.read_initial_money() == 1000);
    }
}

TEST_CASE("Liability parameter getters return values when present and zero when absent", "[core_logic][account][liability]") {
    Liability_parameters params{
        .principal = 100000,
        .liability_interest_rate = 5,
        .compounding_frequency = 12,
        .term = 360,
        .monthly_payment = 600,
        .remaining_balance = 90000,
        .remaining_term = 350,
        .remaining_interest = 20000,
        .remaining_principal = 80000,
        .remaining_total = 100000,
        .credit_limit = 150000,
        .minimum_payment = 200
    };

    SECTION("account with liability parameters exposes them through getters") {
        Account acc_with_liability("Mortgage", Account_type::mortgage, 90000, false, params);

        REQUIRE(acc_with_liability.read_liability_principal() == params.principal);
        REQUIRE(acc_with_liability.read_liability_interest_rate() == params.liability_interest_rate);
        REQUIRE(acc_with_liability.read_liability_compounding_frequency() == params.compounding_frequency);
        REQUIRE(acc_with_liability.read_term() == params.term);
        REQUIRE(acc_with_liability.read_monthly_payment() == params.monthly_payment);
        REQUIRE(acc_with_liability.read_remaining_balance() == params.remaining_balance);
        REQUIRE(acc_with_liability.read_remaining_term() == params.remaining_term);
        REQUIRE(acc_with_liability.read_remaining_interest() == params.remaining_interest);
        REQUIRE(acc_with_liability.read_remaining_principal() == params.remaining_principal);
        REQUIRE(acc_with_liability.read_remaining_total() == params.remaining_total);
        REQUIRE(acc_with_liability.read_credit_limit() == params.credit_limit);
        REQUIRE(acc_with_liability.read_minimum_payment() == params.minimum_payment);
    }

    SECTION("account without liability parameters returns zero for liability getters") {
        Account acc_no_liability("NoLiab", Account_type::mortgage, 0, false);

        REQUIRE(acc_no_liability.read_liability_principal() == 0);
        REQUIRE(acc_no_liability.read_liability_interest_rate() == 0);
        REQUIRE(acc_no_liability.read_liability_compounding_frequency() == 0);
        REQUIRE(acc_no_liability.read_term() == 0);
        REQUIRE(acc_no_liability.read_monthly_payment() == 0);
        REQUIRE(acc_no_liability.read_remaining_balance() == 0);
        REQUIRE(acc_no_liability.read_remaining_term() == 0);
        REQUIRE(acc_no_liability.read_remaining_interest() == 0);
        REQUIRE(acc_no_liability.read_remaining_principal() == 0);
        REQUIRE(acc_no_liability.read_remaining_total() == 0);
        REQUIRE(acc_no_liability.read_credit_limit() == 0);
        REQUIRE(acc_no_liability.read_minimum_payment() == 0);
    }
}

TEST_CASE("Asset parameter getters return values when present and zero when absent", "[core_logic][account][asset]") {
    Asset_parameters params{
        .asset_interest_rate = 7,
        .compounding_frequency = 4
    };

    SECTION("account with asset parameters exposes them through getters") {
        Account acc_with_asset("Investment", Account_type::investments, 50000, true, params);

        REQUIRE(acc_with_asset.read_asset_interest_rate() == params.asset_interest_rate);
        REQUIRE(acc_with_asset.read_asset_compounding_frequency() == params.compounding_frequency);
    }

    SECTION("account without asset parameters returns zero for asset getters") {
        Account acc_no_asset("NoAssetParams", Account_type::investments, 0, true);

        REQUIRE(acc_no_asset.read_asset_interest_rate() == 0);
        REQUIRE(acc_no_asset.read_asset_compounding_frequency() == 0);
    }
}