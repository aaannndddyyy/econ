/****************************************************************

 econ - a simple economics simulator

 =============================================================

 Copyright 2015 Bob Mottram

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the followingp
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

****************************************************************/

#ifndef ECON_H
#define ECON_H

#define MAX_ECONOMY_SIZE         1024

#define LABOUR_TIME_TOTAL        0
#define LABOUR_TIME_NECESSARY    1

#define MIN_WORKING_DAY          8
#define MAX_WORKING_DAY          12

#define MIN_DAYS_PER_WEEK        4
#define MAX_DAYS_PER_WEEK        6

#define MIN_WAGE                 6.70
#define MAX_WAGE                20.00

#define INITIAL_WORKERS          10
#define MIN_WORKERS              4
#define MAX_WORKERS              1000

#define MIN_PRODUCTIVITY         6
#define MAX_PRODUCTIVITY         50

#define MAX_PRODUCT_TYPES        4
#define PROCESS_INPUTS           2
#define PRODUCT_PRIMITIVE        0

#define INITIAL_DEPOSIT          10000
#define INITIAL_MERCHANT_DEPOSIT 10000
#define INITIAL_BANK_DEPOSIT     10000
#define INITIAL_STATE_DEPOSIT    (INITIAL_BANK_DEPOSIT*10)

#define MAX_MERCHANT_STOCK       100000
#define MAX_BANKS                5
#define MAX_ACCOUNTS             (MAX_ECONOMY_SIZE/4)
#define MIN_BANK_INTEREST        0
#define MAX_BANK_INTEREST        30
#define MIN_LOAN_INTEREST        0
#define MAX_LOAN_INTEREST        30

#define MIN_SAVINGS_RATE         0
#define MAX_SAVINGS_RATE         10

#define HISTORY_STEPS            10

/* number of locations/continents */
#define LOCATIONS                3

#define MIN_VAT_RATE             0
#define MAX_VAT_RATE             50

#define MIN_BUSINESS_TAX_RATE    0
#define MAX_BUSINESS_TAX_RATE    50

#define MIN_LOAN                 1000

#define MIN_CITIZENS_DIVIDEND    0
#define MAX_CITIZENS_DIVIDEND    MAX_WAGE

#define MAX_RENTIERS             1024
#define INITIAL_RENTIER_DEPOSIT  10000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

enum {
    ENTITY_NONE,
    ENTITY_FIRM,
    ENTITY_MERCHANT,
    ENTITY_BANK,
    ENTITY_STATE,
    ENTITIES
};

enum {
    ASSET_LAND,
    ASSET_HOUSE,
    ASSET_FACTORY,
    ASSET_TYPES
};

typedef struct
{
    float repayment_per_month;
    float variable, constant;
    float surplus, fictitious;
    float surplus_history[HISTORY_STEPS];
    float savings_rate;
} Capital;

typedef struct
{
    unsigned int days_per_week;
    float time_total;
    float time_necessary;
    unsigned int workers;
    float wage_rate;
    float productivity;
    unsigned char is_recruiting;
} Labour;

typedef struct
{
    unsigned int raw_material[PROCESS_INPUTS];
    float raw_material_stock[PROCESS_INPUTS];
    unsigned int product_type;
    float stock;
} Process;

typedef struct
{
    Capital capital;
    unsigned int tax_location;
    float interest_rate;
    unsigned int hedge;
    float stock[MAX_PRODUCT_TYPES];
    float price[MAX_PRODUCT_TYPES];
} Merchant;

typedef struct
{
    unsigned int location;
    Capital capital;
    Labour labour;
    Process process;
    float sale_value;
} Firm;

typedef struct
{
    unsigned int entity_type;
    unsigned int entity_index;
    float balance;
    float loan;
    float loan_interest_rate;
    unsigned int loan_elapsed_days;
    float loan_repaid;
    float loan_repayment_per_month;
} Account;

typedef struct
{
    Capital capital;
    unsigned int tax_location;
    float interest_deposit;
    float interest_loan;
    unsigned int active_accounts;
    Account account[MAX_ACCOUNTS];
} Bank;

typedef struct
{
    Capital capital;
    unsigned int asset_type;
    float asset_value;
    float rent_per_month;
    unsigned int quantity;
    unsigned int location;
} Rentier;

typedef struct
{
    Capital capital;
    float VAT_rate;
    float business_tax_rate;
    unsigned int population;
    unsigned int unemployed;
    float citizens_dividend;
} State;

typedef struct
{
    unsigned int size;
    Firm firm[MAX_ECONOMY_SIZE];
    Merchant merchant;
    Bank bank[MAX_BANKS];
    State state[LOCATIONS];
    Rentier rentier[MAX_RENTIERS];
    unsigned int bankruptcies;
} Economy;

void clear_history(Capital * c);
void update_history(Capital * c);

float econ_average_price(Economy * e, unsigned int product_type, unsigned int location);
float econ_average_price_global(Economy * e, unsigned int product_type);
int econ_best_price(Economy * e, Firm * f, unsigned int product_type, unsigned int local);
float econ_average_price_variance(Economy * e, unsigned int product_type);

void firm_init(Firm * f);
int firm_defunct(Firm * f);
float firm_worth(Firm * f);
void firm_update(Firm * f, Economy * e, unsigned int weeks);
float firm_working_capital(Firm * f);
void firm_subtract_capital(Firm * f, float amount);

void merchant_init(Merchant * m);
void merchant_update(Economy * e);
float merchant_working_capital(Merchant * m);
void merchant_subtract_capital(Merchant * m, float amount);

void bank_init(Bank * b);
int bank_defunct(Bank * b);
int bank_account_defunct(Account * a);
int bank_account_index(Bank * b, unsigned int entity_type, unsigned int entity_index);
void bank_update(Bank * b, Economy * e, unsigned int increment_days);
void bank_issue_loan(Bank * b, Economy * e,
                     unsigned int entity_type, unsigned int entity_index,
                     float amount, unsigned int repayment_days);
void bank_account_close_entity(Bank * b, Economy * e, unsigned int entity_type, unsigned int entity_index);
float bank_average_interest_loan(Economy * e);
float bank_average_interest_deposit(Economy * e);
float bank_worth(Bank * b);
Bank * best_bank_for_savings(Economy * e);
Bank * best_bank_for_loan(Economy * e);

void state_init(State * s);
void state_update(State * s, Economy * e, unsigned int weeks);
float state_working_capital(State * s);
void state_subtract_capital(State * s, float amount);

void rentier_init(Rentier * r);
void rentier_update(Rentier * r, Economy * e, unsigned int weeks);

#endif
