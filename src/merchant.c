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


void merchant_init(Merchant * m)
{
    unsigned int i;

    m->tax_location = (unsigned int)(rand()%LOCATIONS);
    m->capital.repayment_per_month = 0;
    m->capital.variable = 0;
    m->capital.constant = 10;
    m->capital.surplus = 0;
    m->capital.fictitious = INITIAL_MERCHANT_DEPOSIT;
    m->interest_rate = 2;
    m->hedge = MAX_PRODUCT_TYPES/2;
    for (i = 0; i < MAX_PRODUCT_TYPES; i++) {
        m->stock[i] = 0;
        m->price[i] = 0;
    }
    clear_history(&m->capital);
}

void merchant_buy(Economy * e)
{
    unsigned int i;
    Merchant * m = &e->merchant;
    Firm * f;
    int best_index;
    float investment_tranche = working_capital(&m->capital) / (float)m->hedge;
    float buy_qty, target_price, variance, variance_min=0, variance_max=0;
    float average_variance, value, tax;

    /* calculate price variance range for all commodities */
    for (i = 0; i < MAX_PRODUCT_TYPES; i++) {
        if (m->stock[i] > MAX_MERCHANT_STOCK) continue;
        variance = econ_average_price_variance(e, i);
        if ((variance_max == 0) || (variance > variance_max)) {
            variance_max = variance;
        }
        if ((variance_min == 0) || (variance < variance_min)) {
            variance_min = variance;
        }
    }

    average_variance = variance_min + ((variance_max - variance_min)/2);
    for (i = 0; i < MAX_PRODUCT_TYPES; i++) {
        if (m->stock[i] > MAX_MERCHANT_STOCK) continue;

        /* prefer high variance trades, where you're
           likely to obtain the most return */
        if (econ_average_price_variance(e, i) < average_variance) continue;

        best_index = econ_best_price(e, NULL, i, 0);
        if (best_index == -1) continue;
        f = &e->firm[best_index];
        if (m->price[i] == 0) {
            m->price[i] =
                f->sale_value * (1.0f + (m->interest_rate/100.0f));
        }
        else {
            target_price =
                f->sale_value * (1.0f + (m->interest_rate/100.0f));
            m->price[i] += (target_price - m->price[i])*0.1f;
        }

        buy_qty = investment_tranche / f->sale_value;
        if (buy_qty > 1) {
            if (buy_qty > f->process.stock) {
                buy_qty = f->process.stock;
            }
            if (m->stock[i] + buy_qty > MAX_MERCHANT_STOCK) {
                buy_qty = MAX_MERCHANT_STOCK - m->stock[i];
            }
            if (buy_qty > 1) {
                f->process.stock -= buy_qty;
                if (f->process.stock < 0) f->process.stock = 0;
                m->stock[i] += buy_qty;
                value = f->sale_value * buy_qty;
                tax = value * e->state[f->location].VAT_rate / 100.0f;
                subtract_capital(&m->capital, value);
                f->capital.surplus += value - tax;
            }
        }
    }
}

void merchant_update(Economy * e)
{
    merchant_buy(e);
    update_history(&e->merchant.capital);
}
