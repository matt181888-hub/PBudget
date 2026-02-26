
#include <string>
#include <cstring>
#include "core_logic.h"

int balance_after_transaction(int current_balance_cents, int amount_cents, bool is_deposit, bool is_asset)
{
    int delta = (is_asset == is_deposit) ? amount_cents : -amount_cents;
    return current_balance_cents + delta;
}

Account::Account() : money_amount(0), type_of_account(Account_type::checking), asset(true) {}

Account::Account(Account_type type) : money_amount(0), type_of_account(type),
    asset(type == Account_type::checking || type == Account_type::savings || type == Account_type::investments) {}

Account::Account(int money) : money_amount(money), type_of_account(Account_type::checking), initial_money_amount(money), asset(true) {}

Account::Account(Account_type type, int money) : money_amount(money), type_of_account(type), initial_money_amount(money),
    asset(type == Account_type::checking || type == Account_type::savings || type == Account_type::investments) {}

Account::Account(std::string name, Account_type type, int money, bool is_asset) : money_amount(money), type_of_account(type), account_name(name), initial_money_amount(money), asset(is_asset) {}

Account::Account(std::string name, Account_type type, int money, bool is_asset, Liability_parameters liability_params) : money_amount(money), type_of_account(type), account_name(name), initial_money_amount(money), asset(is_asset), liability_parameters(liability_params) {}

Account::Account(std::string name, Account_type type, int money, bool is_asset, Asset_parameters asset_params) : money_amount(money), type_of_account(type), account_name(name), initial_money_amount(money), asset(is_asset), asset_parameters(asset_params) {}

void Account::modify_account(std::string new_account_name, Account_type new_type_of_account, int new_money)
{
    account_name = new_account_name;
    type_of_account = new_type_of_account;
    money_amount = new_money;
}

int Account::read_liability_principal() const
 {
    if (liability_parameters)
    { 
        return liability_parameters->principal;
    }
    else
    {
        return 0;
    }
}
int Account::read_liability_interest_rate() const
{
    if (liability_parameters)
    {
        return liability_parameters->liability_interest_rate;
    }
    else
    {
        return 0;
    }   
}
int Account::read_liability_compounding_frequency() const
{
    if (liability_parameters)
    {
        return liability_parameters->compounding_frequency;
    }
    else
    {
        return 0;
    }
}
int Account::read_term() const
{
    if (liability_parameters)
    {
        return liability_parameters->term;
    }
    else
    {
        return 0;
    }
}
int Account::read_monthly_payment() const
{
    if (liability_parameters)
    {
        return liability_parameters->monthly_payment;
    }
    else
    {
        return 0;
    }
}
int Account::read_remaining_balance() const
{
    if (liability_parameters)
    {
        return liability_parameters->remaining_balance;
    }
    else
    {
        return 0;
    }
}
int Account::read_remaining_term() const 
{
    if (liability_parameters)
    {
     return liability_parameters->remaining_term;
    }
    else
    {
        return 0;
    }
}
int Account::read_remaining_interest() const
{
    if (liability_parameters)
    {
        return liability_parameters->remaining_interest;
    }
    else
    {
        return 0;
    }
}
int Account::read_remaining_principal() const
{
    if (liability_parameters)
    {
        return liability_parameters->remaining_principal;
    }
    else
    {
        return 0;
    }
}
int Account::read_remaining_total() const
{
    if (liability_parameters)
    {
        return liability_parameters->remaining_total;
    }
    else
    {
        return 0;
    }
}
int Account::read_credit_limit() const
{
    if (liability_parameters)
    {
        return liability_parameters->credit_limit;
    }
    else
    {
        return 0;
    }
}
int Account::read_minimum_payment() const
{
    if (liability_parameters)
    {
        return liability_parameters->minimum_payment;
    }
    else
    {
        return 0;
    }
}

//asset parameter getters
int Account::read_asset_interest_rate() const
{
    if (asset_parameters)
    {
        return asset_parameters->asset_interest_rate;
    }
    else
    {
        return 0;
    }
}
int Account::read_asset_compounding_frequency() const
{
    if (asset_parameters)
    {
        return asset_parameters->compounding_frequency;
    }
    else
    {
        return 0;
    }
}





