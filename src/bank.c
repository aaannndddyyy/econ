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

void bank_init(Bank * b)
{
    unsigned int i;

    b->capital.variable = 0;
    b->capital.constant = 0;
    b->capital.surplus = INITIAL_BANK_CREDIT;
    b->interest_credit =
        MIN_BANK_INTEREST +
        ((rand()%10000/10000.0)*(MAX_BANK_INTEREST - MIN_BANK_INTEREST));
    b->interest_loan =
        b->interest_credit +
        ((rand()%10000/10000.0)*(MAX_LOAN_INTEREST - b->interest_credit));
    for (i = 0; i < MAX_ACCOUNTS; i++) {
        b->account[i].entity_type = ENTITY_NONE;
        b->account[i].entity_index = 0;
        b->account[i].balance = 0;
        b->account[i].loan = 0;
        b->account[i].loan_repaid = 0;
        b->account[i].loan_increment = 0;
    }
}

int bank_account_defunct(Account * a)
{
    return (a->entity_index == ENTITY_NONE);
}
