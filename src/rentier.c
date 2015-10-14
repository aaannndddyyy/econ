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

void rentier_init(Rentier * r)
{
    r->capital.surplus = INITIAL_RENTIER_DEPOSIT;
    r->capital.variable = 0;
    r->capital.constant = 0;
    r->capital.repayment_per_month = 0;
    r->capital.savings_rate = 0;
    clear_history(&r->capital);
    r->location = (unsigned int)(rand()%LOCATIONS);
    r->asset_type = (unsigned int) (rand()%ASSET_TYPES);
    r->quantity = 0;
    r->asset_value = 0;
    r->rent_per_month = 0;
}

int rentier_index(Rentier * r, Economy * e)
{
    unsigned int i;
    Rentier * r2;

    for (i = 0; i < MAX_RENTIERS; i++) {
        r2 = &e->rentier[i];
        if (r2 == r) return (int)i;
    }
    return -1;
}

void rentier_update(Rentier * r, Economy * e, unsigned int weeks)
{
    /* obtaining loans */
    if (r->capital.repayment_per_month == 0) {
    }

    update_history(&r->capital);
}
