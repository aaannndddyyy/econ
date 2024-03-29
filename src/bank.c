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

/* references:
   http://www.bankofengland.co.uk/publications/Documents/quarterlybulletin/2014/qb14q1prereleasemoneycreation.pdf
*/

#include "econ.h"

void bank_init(Bank * b)
{
    unsigned int i;

    b->tax_location = (unsigned int)(rand()%LOCATIONS);
    b->capital.repayment_per_month = 0;
    b->capital.variable = 0;
    b->capital.constant = 0;
    b->capital.surplus = 0;
    b->capital.fictitious = INITIAL_BANK_DEPOSIT;
    b->interest_deposit =
        MIN_BANK_INTEREST +
        ((rand()%10000/10000.0)*(MAX_BANK_INTEREST - MIN_BANK_INTEREST));
    b->interest_loan =
        b->interest_deposit +
        ((rand()%10000/10000.0)*(MAX_LOAN_INTEREST - b->interest_deposit));
    b->active_accounts = 0;
    for (i = 0; i < MAX_ACCOUNTS; i++) {
        b->account[i].entity_type = ENTITY_NONE;
        b->account[i].entity_index = 0;
        b->account[i].balance = 0;
        b->account[i].loan = 0;
        b->account[i].loan_interest_rate = 0;
        b->account[i].loan_elapsed_days = 0;
        b->account[i].loan_repaid = 0;
        b->account[i].loan_repayment_per_month = 0;
    }
	clear_history(&b->capital);
}

int bank_account_defunct(Account * a)
{
    return (a->entity_index == ENTITY_NONE);
}

float bank_worth(Bank * b)
{
    unsigned int i;
    Account * a;
    float total = b->capital.surplus + b->capital.fictitious;

    for (i = 0; i < MAX_ACCOUNTS; i++) {
        a = &b->account[i];
        if (bank_account_defunct(a)) continue;
        total += a->loan - a->balance;
    }
    return total;
}

int bank_defunct(Bank * b)
{
    return (bank_worth(b) < 0);
}

int bank_account_index(Bank * b, unsigned int entity_type, unsigned int entity_index)
{
    unsigned int i;
    Account * a;

    if (b->active_accounts == 0) return -1;

    for (i = 0; i < MAX_ACCOUNTS; i++) {
        a = &b->account[i];
        if (bank_account_defunct(a)) continue;
        if (a->entity_type != entity_type) continue;
        if (a->entity_index != entity_index) continue;
        return (int)i;
    }
    return -1;
}

/* compound interest equation */
float bank_loan_due(Account * a)
{
    float v, interest_rate = a->loan_interest_rate/100.0f;
    v = 1.0f + (interest_rate/365.0f);
    return a->loan * pow(v, 365.0f * a->loan_elapsed_days);
}

void bank_issue_loan(Bank * b, Economy * e,
                     unsigned int entity_type, unsigned int entity_index,
                     float amount, unsigned int repayment_days)
{
    int account_index;
    float repayment_per_month;
    unsigned int i;
    Firm * firm_borrowing;
    Bank * bank_borrowing;
    State * state_borrowing;

    account_index = bank_account_index(b, entity_type, entity_index);
    if (account_index == -1) {
        if (b->active_accounts >= MAX_ACCOUNTS) return;
        for (i = 0; i < b->active_accounts; i++) {
            if (bank_account_defunct(&b->account[i])) {
                account_index = (int)i;
                break;
            }
        }
        if ((account_index == -1) &&
            (b->active_accounts < MAX_ACCOUNTS-1)) {
            account_index = (int)b->active_accounts;
            b->active_accounts++;
        }
    }
    if (account_index == -1) {
        return;
    }
    if (b->account[account_index].loan > 0) return;

    repayment_per_month = amount * 2 / ((float)repayment_days/30.0f);

    b->capital.fictitious -= amount;
    b->account[account_index].entity_type = entity_type;
    b->account[account_index].entity_index = entity_index;
    b->account[account_index].balance = 0;
    b->account[account_index].loan = amount;
    b->account[account_index].loan_interest_rate = b->interest_loan;
    b->account[account_index].loan_elapsed_days = 0;
    b->account[account_index].loan_repaid = 0;
    b->account[account_index].loan_repayment_per_month = repayment_per_month;

    switch(entity_type) {
    case ENTITY_FIRM: {
        firm_borrowing = &e->firm[entity_index];
        firm_borrowing->capital.repayment_per_month = repayment_per_month;
        firm_borrowing->capital.fictitious += amount;
        break;
    }
    case ENTITY_BANK: {
        bank_borrowing = &e->bank[entity_index];
        bank_borrowing->capital.repayment_per_month = repayment_per_month;
        bank_borrowing->capital.fictitious += amount;
        break;
    }
    case ENTITY_STATE: {
        state_borrowing = &e->state[entity_index];
        state_borrowing->capital.repayment_per_month = repayment_per_month;
        state_borrowing->capital.fictitious += amount;
        break;
    }
    }
}

void bank_loan_close(Bank * b, Economy * e, Account * a)
{
    Firm * firm_borrowing;
    Bank * bank_borrowing;
    State * state_borrowing;

    if (bank_account_defunct(a)) return;

    switch(a->entity_type) {
    case ENTITY_FIRM: {
        firm_borrowing = &e->firm[a->entity_index];
        firm_borrowing->capital.repayment_per_month = 0;
        break;
    }
    case ENTITY_BANK: {
        bank_borrowing = &e->bank[a->entity_index];
        bank_borrowing->capital.repayment_per_month = 0;
        break;
    }
    case ENTITY_STATE: {
        state_borrowing = &e->state[a->entity_index];
        state_borrowing->capital.repayment_per_month = 0;
        break;
    }
    }

    a->loan = 0;
    a->loan_repaid = 0;
    a->loan_repayment_per_month = 0;
}

void bank_account_close_entity(Bank * b, Economy * e, unsigned int entity_type, unsigned int entity_index)
{
    Account * a;
    unsigned int i;

    for (i = 0; i < MAX_ACCOUNTS; i++) {
        a = &b->account[i];
        if (bank_account_defunct(a)) continue;
        if ((a->entity_type == entity_type) &&
            (a->entity_index == entity_index)) {
            bank_loan_close(b, e, a);
            a->entity_index = ENTITY_NONE;
            a->entity_index = 0;
            b->active_accounts--;
        }
    }
}

void bank_loan_repay(Bank * b, Economy * e, Account * a, unsigned int increment_days)
{
    Firm * firm_borrowing;
    Bank * bank_borrowing;
    State * state_borrowing;
    float repayment = (float)increment_days * a->loan_repayment_per_month / 30.0f;

    switch(a->entity_type) {
    case ENTITY_FIRM: {
        firm_borrowing = &e->firm[a->entity_index];
        firm_borrowing->capital.surplus -= repayment;
		b->capital.surplus += repayment;
        break;
    }
    case ENTITY_BANK: {
        bank_borrowing = &e->bank[a->entity_index];
        bank_borrowing->capital.fictitious -= repayment;
		b->capital.fictitious += repayment;
        break;
    }
    case ENTITY_STATE: {
        state_borrowing = &e->state[a->entity_index];
        state_borrowing->capital.surplus -= repayment;
		b->capital.surplus += repayment;
        break;
    }
    }
    a->loan_repaid += repayment;
}

void bank_account_update(Bank * b, Economy * e, unsigned int account_index, unsigned int increment_days)
{
    Account * a = &b->account[account_index];
    if (bank_account_defunct(a)) return;

    if (a->balance > 0) {
        a->balance *= (1.0f + (b->interest_deposit/100.0f));
    }
    if (a->loan > 0) {
        a->loan_elapsed_days += increment_days;

        bank_loan_repay(b, e, a, increment_days);

        if (a->loan_repaid >= bank_loan_due(a)) {
            bank_loan_close(b, e, a);
        }
    }
}

float bank_average_interest_loan(Economy * e)
{
    unsigned int i, hits=0;
    Bank * b;
    float average = 0;

    for (i = 0; i < MAX_BANKS; i++) {
        b = &e->bank[i];
        if (bank_defunct(b)) continue;
        average += b->interest_loan;
        hits++;
    }
    if (hits > 0) {
        average /= (float)hits;
    }
    return average;
}

float bank_average_worth(Economy * e)
{
    unsigned int i, hits=0;
    Bank * b;
    float average = 0;

    for (i = 0; i < MAX_BANKS; i++) {
        b = &e->bank[i];
        if (bank_defunct(b)) continue;
        average += bank_worth(b);
        hits++;
    }
    if (hits > 0) {
        average /= (float)hits;
    }
    return average;
}

float bank_average_interest_deposit(Economy * e)
{
    unsigned int i, hits=0;
    Bank * b;
    float average = 0;

    for (i = 0; i < MAX_BANKS; i++) {
        b = &e->bank[i];
        if (bank_defunct(b)) continue;
        average += b->interest_deposit;
        hits++;
    }
    if (hits > 0) {
        average /= (float)hits;
    }
    return average;
}

void bank_strategy(Bank * b, Economy * e)
{
    float average = bank_average_interest_loan(e);
    if (b->interest_loan > average * 1.05f) {
        b->interest_loan *= 0.99f;
    }
    if (b->interest_loan < average * 0.95f) {
        b->interest_loan *= 1.01f;
    }

    average = bank_average_interest_loan(e);
    if (b->interest_deposit > average * 1.05f) {
        b->interest_deposit *= 0.99f;
    }
    if (b->interest_deposit < average * 0.95f) {
        b->interest_deposit *= 1.01f;
    }

    average = bank_average_worth(e);
    if (bank_worth(b) > average) {
        b->interest_loan *= 1.01f;
        b->interest_deposit *= 0.99f;
    }
    else {
        b->interest_loan *= 0.99f;
        b->interest_deposit *= 1.01f;
    }
}

void bank_update(Bank * b, Economy * e, unsigned int increment_days)
{
    unsigned int i;
    Account * a;

    if (bank_defunct(b)) return;

    for (i = 0; i < MAX_ACCOUNTS; i++) {
        bank_account_update(b, e, i, increment_days);
    }

    bank_strategy(b, e);
    update_history(&b->capital);

    if (bank_defunct(b)) {
        for (i = 0; i < MAX_ACCOUNTS; i++) {
            a = &b->account[i];
            bank_account_close_entity(b, e, a->entity_type, a->entity_index);
        }
        e->bankruptcies++;
    }
}

Bank * best_bank_for_loan(Economy * e)
{
    unsigned int i;
    Bank * b, * best = NULL;
    float min_interest_rate = 0;

    for (i = 0; i < MAX_BANKS; i++) {
        b = &e->bank[i];
        if (bank_defunct(b)) continue;
        if ((best == NULL) || (b->interest_loan < min_interest_rate)) {
            min_interest_rate = b->interest_loan;
            best = b;
        }
    }
    return best;
}

Bank * best_bank_for_savings(Economy * e)
{
    unsigned int i;
    Bank * b, * best = NULL;
    float max_interest_rate = 0;

    for (i = 0; i < MAX_BANKS; i++) {
        b = &e->bank[i];
        if (bank_defunct(b)) continue;
        if ((best == NULL) || (b->interest_deposit > max_interest_rate)) {
            max_interest_rate = b->interest_deposit;
            best = b;
        }
    }
    return best;
}
