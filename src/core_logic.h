#pragma once
#include <string>
#include <cstring>
#include <ctime>
#include <optional>



enum class Account_type
{
    checking,
    savings,
    investments,
    credit_card,
    loan,
    mortgage,
    other
};

enum class Transaction_type
{
    Need,
    Want,
    Savings,
    Internal_transfer,
    Income,
    Gift,
    Dividends,
    Other
};

enum class Transaction_category_need
{
    Housing,
    Food,
    Transportation,
    Utilities,
    Healthcare,
    Debt,
    Dependants,
    Other
};

enum class Transaction_category_want
{
    Shopping,
    Entertainment,
    Eating_out,
    Travel,
    Leisure,
    Gifts,
    Other
};

/* Transaction table schema (see storage.cpp): id, account_id, transaction_amount (INTEGER cents),
   transaction_type TEXT, previous_amount/new_amount (INTEGER), transaction_date (INTEGER unix), transaction_name, note */

class Transaction_info
{
    // data describing a single transaction for an account
    // (account_id is handled in the Storage layer)
    public:
        int transaction_id;
        int account_id;
        int transaction_amount;
        Transaction_type type_of_transaction;
        Transaction_category_need transaction_category_need;
        Transaction_category_want transaction_category_want;
        int account_previous_amount;     // balance before this transaction (in cents)
        int account_new_amount;          // balance after this transaction (in cents)
        std::time_t ymd;                 // when the transaction occurred (Unix timestamp)
        std::string transaction_name;    // short label/name shown in UI
        std::string note;                // optional longer description
};

struct Liability_parameters
{
    int principal;
    int liability_interest_rate;
    int compounding_frequency;
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

struct Asset_parameters
{
    int asset_interest_rate;
    int compounding_frequency;
};

/** Domain rule: compute new balance after a deposit or withdrawal.
 *  For assets: deposit increases balance, withdraw decreases it.
 *  For liabilities: deposit (e.g. payment) decreases balance, withdraw (e.g. charge) increases it.
 *  amount_cents is the user-entered positive amount. */
int balance_after_transaction(int current_balance_cents, int amount_cents, bool is_deposit, bool is_asset);

class Account
{
    public:

        //Constructors
        Account(std::string account_name, Account_type type, int money, bool is_asset);
        Account(std::string name, Account_type type, int money, bool is_asset, Liability_parameters liability_params);
        Account(std::string name, Account_type type, int money, bool is_asset, Asset_parameters asset_params);

        //methods
        void modify_account(std::string account_name, Account_type type_of_account, int money);

        int read_money() const {return money_amount;}
        int read_initial_money() const {return initial_money_amount;}
        Account_type read_account_type() const {return type_of_account;}
        std::string read_account_name() const {return account_name;}
        void set_account_id(int database_row_id) {account_id_in_DB = database_row_id;}
        int read_account_id_in_DB() const {return account_id_in_DB;}
        bool is_asset() const {return asset;}

        //liability parameter_getters
        int read_liability_principal() const;
        int read_liability_interest_rate() const;
        int read_liability_compounding_frequency() const;
        int read_term() const;
        int read_monthly_payment() const;
        int read_remaining_balance() const;
        int read_remaining_term() const;
        int read_remaining_interest() const;    
        int read_remaining_principal() const;
        int read_remaining_total() const;
        int read_credit_limit() const;
        int read_minimum_payment() const;

        //asset parameter getters
        int read_asset_interest_rate() const;
        int read_asset_compounding_frequency() const;

    private:
        //data
        int money_amount = 0;
        int initial_money_amount = 0;
        int account_id_in_DB = 0;
        Account_type type_of_account;
        std::string account_name;
        std::optional<Liability_parameters> liability_parameters;
        std::optional<Asset_parameters> asset_parameters;
        bool asset;
};



