#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/helpers.h"
#include <cstring>

// Layer 2: helpers unit tests

TEST_CASE("account_type dropdown and combo index are consistent", "[helpers][account_type]") {
    SECTION("dropdown index maps to expected enum") {
        REQUIRE(account_type_from_dropdown(0) == Account_type::checking);
        REQUIRE(account_type_from_dropdown(1) == Account_type::savings);
        REQUIRE(account_type_from_dropdown(2) == Account_type::investments);
        REQUIRE(account_type_from_dropdown(3) == Account_type::credit_card);
        REQUIRE(account_type_from_dropdown(4) == Account_type::loan);
        REQUIRE(account_type_from_dropdown(5) == Account_type::mortgage);
        REQUIRE(account_type_from_dropdown(6) == Account_type::other);
    }

    SECTION("round-trip between enum and combo index works") {
        for (int i = 0; i <= 6; ++i) {
            Account_type t = account_type_from_dropdown(i);
            int combo_idx = combo_index_from_account_type(t);
            REQUIRE(combo_idx == i);
        }
    }

    SECTION("asset classification matches expected types") {
        REQUIRE(account_type_is_asset(Account_type::checking));
        REQUIRE(account_type_is_asset(Account_type::savings));
        REQUIRE(account_type_is_asset(Account_type::investments));

        REQUIRE_FALSE(account_type_is_asset(Account_type::credit_card));
        REQUIRE_FALSE(account_type_is_asset(Account_type::loan));
        REQUIRE_FALSE(account_type_is_asset(Account_type::mortgage));
        REQUIRE_FALSE(account_type_is_asset(Account_type::other));
    }
}

TEST_CASE("account type string conversions are symmetric and robust", "[helpers][account_type]") {
    SECTION("known account types round-trip through string") {
        Account_type types[] = {
            Account_type::checking,
            Account_type::savings,
            Account_type::investments,
            Account_type::credit_card,
            Account_type::loan,
            Account_type::mortgage,
            Account_type::other
        };

        for (Account_type t : types) {
            const char* s = account_type_to_string(t);
            Account_type parsed = account_type_from_string(s);
            REQUIRE(parsed == t);
        }
    }

    SECTION("unknown or null strings fall back safely") {
        REQUIRE(account_type_from_string(nullptr) == Account_type::checking);
        REQUIRE(account_type_from_string("NotARealType") == Account_type::checking);
    }

    SECTION("out-of-range dropdown indices fall back to other") {
        REQUIRE(account_type_from_dropdown(-1) == Account_type::other);
        REQUIRE(account_type_from_dropdown(999) == Account_type::other);
    }
}

TEST_CASE("compounding index and frequency mappings stay in sync", "[helpers][compounding]") {
    SECTION("combo index to frequency") {
        REQUIRE(compounding_frequency_from_index(0) == 365);
        REQUIRE(compounding_frequency_from_index(1) == 12);
        REQUIRE(compounding_frequency_from_index(2) == 1);
        REQUIRE(compounding_frequency_from_index(42) == 1);
    }

    SECTION("frequency to combo index") {
        REQUIRE(compounding_index_from_frequency(365) == 0);
        REQUIRE(compounding_index_from_frequency(12) == 1);
        REQUIRE(compounding_index_from_frequency(1) == 2);
        REQUIRE(compounding_index_from_frequency(999) == 2);
    }
}

TEST_CASE("cents_to_dollars converts integer cents to float dollars", "[helpers][money]") {
    REQUIRE(cents_to_dollars(0) == Catch::Approx(0.0f));
    REQUIRE(cents_to_dollars(1) == Catch::Approx(0.01f));
    REQUIRE(cents_to_dollars(123) == Catch::Approx(1.23f));
    REQUIRE(cents_to_dollars(-50) == Catch::Approx(-0.50f));
}

TEST_CASE("cents_to_dollars handles large and edge amounts", "[helpers][money][edge]") {
    REQUIRE(cents_to_dollars(123456789) == Catch::Approx(1234567.89f));
    REQUIRE(cents_to_dollars(-99999) == Catch::Approx(-999.99f));
}

TEST_CASE("transaction type dropdown mappings cover all UI options", "[helpers][transaction_type]") {
    SECTION("deposit transaction type from dropdown") {
        REQUIRE(deposit_transaction_type_from_dropdown(0) == Transaction_type::Income);
        REQUIRE(deposit_transaction_type_from_dropdown(1) == Transaction_type::Gift);
        REQUIRE(deposit_transaction_type_from_dropdown(2) == Transaction_type::Dividends);
        REQUIRE(deposit_transaction_type_from_dropdown(3) == Transaction_type::Other);
        REQUIRE(deposit_transaction_type_from_dropdown(99) == Transaction_type::Other);
    }

    SECTION("withdrawal transaction type from dropdown") {
        REQUIRE(withdrawal_transaction_type_from_dropdown(0) == Transaction_type::Need);
        REQUIRE(withdrawal_transaction_type_from_dropdown(1) == Transaction_type::Want);
        REQUIRE(withdrawal_transaction_type_from_dropdown(2) == Transaction_type::Savings);
        REQUIRE(withdrawal_transaction_type_from_dropdown(3) == Transaction_type::Other);
        REQUIRE(withdrawal_transaction_type_from_dropdown(99) == Transaction_type::Other);
    }
}

TEST_CASE("transaction type string conversions are symmetric and handle unknowns", "[helpers][transaction_type]") {
    SECTION("known transaction types round-trip through string") {
        Transaction_type types[] = {
            Transaction_type::Need,
            Transaction_type::Want,
            Transaction_type::Savings,
            Transaction_type::Internal_transfer,
            Transaction_type::Income,
            Transaction_type::Gift,
            Transaction_type::Dividends,
            Transaction_type::Other
        };

        for (Transaction_type t : types) {
            const char* s = transaction_type_to_string(t);
            Transaction_type parsed = transaction_type_from_string(s);
            REQUIRE(parsed == t);
        }
    }

    SECTION("unknown or null strings map to Other") {
        REQUIRE(transaction_type_from_string(nullptr) == Transaction_type::Other);
        REQUIRE(transaction_type_from_string("NotARealType") == Transaction_type::Other);
    }

    SECTION("transaction type string mapping is case-sensitive and strict") {
        REQUIRE(transaction_type_from_string("need") == Transaction_type::Other);
        REQUIRE(transaction_type_from_string("NEED") == Transaction_type::Other);
        REQUIRE(transaction_type_from_string("Need ") == Transaction_type::Other);
        REQUIRE(transaction_type_from_string("Need") == Transaction_type::Need);
    }

    SECTION("Internal_transfer uses Internal_transfer string not Transfer") {
        REQUIRE(std::strcmp(transaction_type_to_string(Transaction_type::Internal_transfer), "Internal_transfer") == 0);
        REQUIRE(transaction_type_from_string("Internal_transfer") == Transaction_type::Internal_transfer);
        REQUIRE(transaction_type_from_string("Transfer") == Transaction_type::Other);
    }
}

TEST_CASE("transaction category dropdown mappings are correct", "[helpers][transaction_category]") {
    SECTION("need category from dropdown") {
        REQUIRE(transaction_category_need_from_dropdown(0) == Transaction_category_need::Housing);
        REQUIRE(transaction_category_need_from_dropdown(6) == Transaction_category_need::Dependants);
        REQUIRE(transaction_category_need_from_dropdown(7) == Transaction_category_need::Other);
    }

    SECTION("want category from dropdown") {
        REQUIRE(transaction_category_want_from_dropdown(0) == Transaction_category_want::Shopping);
        REQUIRE(transaction_category_want_from_dropdown(5) == Transaction_category_want::Gifts);
        REQUIRE(transaction_category_want_from_dropdown(6) == Transaction_category_want::Other);
    }

    SECTION("negative or out-of-range category dropdown indices fall back to Other") {
        REQUIRE(transaction_category_need_from_dropdown(-1) == Transaction_category_need::Other);
        REQUIRE(transaction_category_need_from_dropdown(999) == Transaction_category_need::Other);
        REQUIRE(transaction_category_want_from_dropdown(-1) == Transaction_category_want::Other);
        REQUIRE(transaction_category_want_from_dropdown(999) == Transaction_category_want::Other);
    }
}

TEST_CASE("transaction category string conversions are symmetric and robust", "[helpers][transaction_category]") {
    SECTION("need category string conversions") {
        Transaction_category_need n = Transaction_category_need::Healthcare;
        const char* s = transaction_category_need_to_string(n);
        Transaction_category_need parsed = transaction_category_need_from_string(s);
        REQUIRE(parsed == n);

        REQUIRE(transaction_category_need_from_string(nullptr) == Transaction_category_need::Other);
        REQUIRE(transaction_category_need_from_string("NotARealNeed") == Transaction_category_need::Other);
    }

    SECTION("want category string conversions") {
        Transaction_category_want w = Transaction_category_want::Travel;
        const char* s = transaction_category_want_to_string(w);
        Transaction_category_want parsed = transaction_category_want_from_string(s);
        REQUIRE(parsed == w);

        REQUIRE(transaction_category_want_from_string(nullptr) == Transaction_category_want::Other);
        REQUIRE(transaction_category_want_from_string("NotARealWant") == Transaction_category_want::Other);
    }
}

TEST_CASE("create_transaction_info populates all fields correctly", "[helpers][transaction_info]") {
    int account_id = 42;
    int amount = 12345;
    Transaction_type type = Transaction_type::Need;
    Transaction_category_need need_cat = Transaction_category_need::Housing;
    Transaction_category_want want_cat = Transaction_category_want::Shopping;
    std::string name = "Rent payment";
    std::string note = "January rent";
    int prev_amount = 500000;
    int new_amount = 487655;

    std::time_t before = std::time(nullptr);
    Transaction_info info = create_transaction_info(
        account_id,
        amount,
        type,
        need_cat,
        want_cat,
        name,
        note,
        prev_amount,
        new_amount
    );
    std::time_t after = std::time(nullptr);

    REQUIRE(info.account_id == account_id);
    REQUIRE(info.transaction_amount == amount);
    REQUIRE(info.type_of_transaction == type);
    REQUIRE(info.transaction_category_need == need_cat);
    REQUIRE(info.transaction_category_want == want_cat);
    REQUIRE(info.transaction_name == name);
    REQUIRE(info.note == note);
    REQUIRE(info.account_previous_amount == prev_amount);
    REQUIRE(info.account_new_amount == new_amount);
    REQUIRE(info.ymd >= before);
    REQUIRE(info.ymd <= after);
}

TEST_CASE("create_transaction_info handles empty and minimal inputs", "[helpers][transaction_info][edge]") {
    Transaction_info info = create_transaction_info(
        1,
        0,
        Transaction_type::Other,
        Transaction_category_need::Other,
        Transaction_category_want::Other,
        "",
        "",
        1000,
        1000
    );

    REQUIRE(info.account_id == 1);
    REQUIRE(info.transaction_amount == 0);
    REQUIRE(info.type_of_transaction == Transaction_type::Other);
    REQUIRE(info.transaction_category_need == Transaction_category_need::Other);
    REQUIRE(info.transaction_category_want == Transaction_category_want::Other);
    REQUIRE(info.transaction_name == "");
    REQUIRE(info.note == "");
    REQUIRE(info.account_previous_amount == 1000);
    REQUIRE(info.account_new_amount == 1000);
    REQUIRE(info.ymd != 0);
}

