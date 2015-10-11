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

void econ_init(Economy * e)
{
    unsigned int i;

    e->size = MAX_ECONOMY_SIZE;
    e->unemployed = 0;
    e->population = 0;
    e->bankruptcies = 0;
    for (i = 0; i < e->size; i++) {
        firm_init(&e->firm[i]);
        e->population += e->firm[i].labour.workers;
    }
    merchant_init(&e->merchant);
    bank_init(&e->bank);
}

float econ_average_price(Economy * e, unsigned int product_type, unsigned int location)
{
    unsigned int i,hits=0;
    Firm * f;
    float average = 0;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        if ((f->process.product_type == product_type) &&
            (f->process.stock > 0) &&
            (f->location == location)) {
            average += f->sale_value*f->process.stock;
            hits += f->process.stock;
        }
    }

    average += e->merchant.price[product_type] * e->merchant.stock[product_type];
    hits += e->merchant.stock[product_type];

    if (hits > 0) return average / (float)hits;
    return 0;
}

float econ_average_price_global(Economy * e, unsigned int product_type)
{
    unsigned int i,hits=0;
    Firm * f;
    float average = 0;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        if ((f->process.product_type == product_type) &&
            (f->process.stock > 0)) {
            average += f->sale_value*f->process.stock;
            hits += f->process.stock;
        }
    }

    if (hits > 0) return average / (float)hits;
    return 0;
}

float econ_average_price_variance(Economy * e, unsigned int product_type)
{
    unsigned int i,hits=0;
    Firm * f;
    float average = econ_average_price_global(e, product_type);
    float variance = 0;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        if ((f->process.product_type == product_type) &&
            (f->process.stock > 0)) {
            variance += (f->sale_value - average)*(f->sale_value - average);
            hits++;
        }
    }

    if (hits > 0) return (float)sqrt(average / (float)hits);
    return 0;
}

float econ_average_wage(Economy * e, unsigned int location)
{
    unsigned int i,hits=0;
    Firm * f;
    float average = 0;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        if (f->location == location) {
            average += f->labour.wage_rate;;
            hits++;
        }
    }
    if (hits > 0) return average / (float)hits;
    return 0;
}

void econ_startups(Economy * e)
{
    unsigned int i;
    Firm * f;

    if (e->unemployed < INITIAL_WORKERS) return;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) {
            if (e->unemployed >= INITIAL_WORKERS) {
                firm_init(f);
                e->unemployed -= f->labour.workers;
                if (e->bankruptcies > 0) e->bankruptcies--;
            }
        }
    }
}

void econ_mergers(Economy * e)
{
    unsigned int i, j;
    int best_index;
    Firm * f, * f2;
    float best;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        best_index = -1;
        best = 0;
        for (j = 0; j < e->size; j++) {
            if (i == j) continue;
            f2 = &e->firm[j];
            if (f2->labour.workers == 0) continue;
            if (f2->location != f->location) continue;
            if (f->capital.surplus > firm_worth(f2)) {
                if (f->labour.workers + f2->labour.workers < MAX_WORKERS) {
                    if (firm_worth(f2) > best) {
                        best_index = (int)j;
                        best = firm_worth(f2);
                    }
                }
            }
        }
        if (best_index > -1) {
            f2 = &e->firm[best_index];
            f->capital.surplus -= best;
            f->labour.workers += f2->labour.workers;
            f2->labour.workers = 0;
        }
    }
}

void econ_bankrupt(Economy * e)
{
    unsigned int i;
    Firm * f;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        if (f->capital.surplus < 0) {
            e->unemployed += f->labour.workers;
            f->labour.workers = 0;
            e->bankruptcies++;
        }
    }
}

void econ_labour_market(Economy * e)
{
    unsigned int i, j, max, recruiting=0;
    int best;
    Firm * f, * f2;
    float max_wage;

    /* workers can move between firms */
    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        max_wage = f->labour.wage_rate;
        best = -1;
        for (j = 0; j < e->size; j++) {
            if (i == j) continue;
            f2 = &e->firm[j];
            if (f2->labour.workers == 0) continue;

            if ((f2->labour.wage_rate > max_wage) &&
                (f2->labour.workers < MAX_WORKERS-1)) {
                max_wage = f2->labour.wage_rate;
                best = (int)j;
            }
        }
        if (best > -1) {
            f2 = &e->firm[best];
            f->labour.workers--;
            f2->labour.workers++;
            f2->labour.is_recruiting = 0;
        }
    }

    /* unemployed may be recruited */
    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        if (f->labour.is_recruiting == 1) recruiting++;
    }
    if (recruiting > 0) {
        max = e->unemployed;
        for (i = 0; i < max; i++) {
            max_wage = 0;
            best = -1;
            for (j = 0; j < e->size; j++) {
                f = &e->firm[i];
                if (firm_defunct(f)) continue;
                if (f->labour.is_recruiting == 0) continue;
                if (f->labour.wage_rate > max_wage) {
                    max_wage = f->labour.wage_rate;
                    best = (int)j;
                }
            }
            if (best > -1) {
                f = &e->firm[best];
                f->labour.workers++;
                f->labour.is_recruiting = 0;
                recruiting--;
                e->unemployed--;
            }
            if (recruiting == 0) break;
        }
    }
}

/* returns the index of the firm with the best offering price for a commodity */
int econ_best_price(Economy * e, Firm * f, unsigned int product_type, unsigned int local)
{
    unsigned int i;
    int best_index = -1;
    Firm * f2;
    float best = 0;

    for (i = 0; i < e->size; i++) {
        f2 = &e->firm[i];
        if (firm_defunct(f2)) continue;
        if (f != NULL) {
            if (f2 == f) continue;
            if ((local != 0) && (f2->location != f->location)) continue;
        }
        if ((f2->process.product_type == product_type) &&
            (f2->process.stock > 0)) {
            if ((best_index == -1) || (f2->sale_value < best)) {
                best = f2->sale_value;
                best_index = i;
            }
        }
    }
    return best_index;
}

void econ_update(Economy * e, unsigned int weeks)
{
    unsigned int i;
    Firm * f;

    econ_startups(e);
    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        firm_update(f, e, weeks);
    }
    merchant_update(e);
    econ_bankrupt(e);
    econ_mergers(e);
    econ_labour_market(e);
}

int main(int argc, char* argv[])
{
    Economy e;
    unsigned int i, j;

    econ_init(&e);

    for (i = 0; i < 100; i++)  {
        econ_update(&e, 1);
        printf("Profit: %.2f\n",e.firm[0].capital.surplus);
        printf("Bankrupt: %d/%d\n",e.bankruptcies,e.size);
        printf("Unemployed: %d/%d\n",e.unemployed,e.population);
        printf("Merchant: ");
        for (j = 0; j < MAX_PRODUCT_TYPES; j++)  {
            printf("%d ", (int)e.merchant.stock[j]);
        }
        printf("\n");
    }
    return 0;
}
