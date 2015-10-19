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

#include "econ.h"

void state_init(State * s)
{
    s->capital.fictitious = INITIAL_STATE_DEPOSIT;
    s->capital.surplus = 0;
    s->capital.variable = 0;
    s->capital.constant = 0;
    s->capital.repayment_per_month = 0;
    s->capital.savings_rate = 0;
    clear_history(&s->capital);
    s->population = INITIAL_WORKERS;
    s->unemployed = 0;
    s->VAT_rate = MIN_VAT_RATE +
        ((rand()%10000/10000.0)*(MAX_VAT_RATE - MIN_VAT_RATE));
    s->business_tax_rate = MIN_BUSINESS_TAX_RATE +
        ((rand()%10000/10000.0f)*(MAX_BUSINESS_TAX_RATE - MIN_BUSINESS_TAX_RATE));
    s->citizens_dividend = MIN_CITIZENS_DIVIDEND +
        ((rand()%10000/10000.0f)*(MAX_CITIZENS_DIVIDEND - MIN_CITIZENS_DIVIDEND));
}

float state_working_capital(State * s)
{
    return s->capital.surplus + s->capital.fictitious;
}

void state_subtract_capital(State * s, float amount)
{
    if (amount <= s->capital.surplus) {
        s->capital.surplus -= amount;
        return;
    }
    amount -= s->capital.surplus;
    s->capital.surplus = 0;
    s->capital.fictitious -= amount;
}

float state_spending(State * s, unsigned int weeks)
{
    /* citizen's dividend is hourly, like wages */
    float welfare = (s->population * s->citizens_dividend) * 24 * 7 * (float)weeks;
    float borrowing = s->capital.repayment_per_month * (float)weeks / 4;
    return welfare + borrowing;
}

int state_index(State * s, Economy * e)
{
    unsigned int i;
    State * s2;

    for (i = 0; i < LOCATIONS; i++) {
        s2 = &e->state[i];
        if (s2 == s) return (int)i;
    }
    return -1;
}

void state_update(State * s, Economy * e, unsigned int weeks)
{
    Bank * best;
    unsigned int index, repayment_days;
    float amount;

    /* obtaining loans */
    if (s->capital.repayment_per_month == 0) {
        if (state_spending(s, weeks) > state_working_capital(s)) {
            best = best_bank_for_loan(e);
            if (best != NULL) {
                index = state_index(s, e);
                amount = state_spending(s, weeks)*2;
                repayment_days = 7 * weeks * 3;
                bank_issue_loan(best, e, ENTITY_STATE,
                                (unsigned int)index,
                                amount, repayment_days);
            }
        }
    }

    /* spending */
    state_subtract_capital(s, state_spending(s, weeks));
    update_history(&s->capital);
}
