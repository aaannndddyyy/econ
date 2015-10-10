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

void firm_init_process(Firm * f)
{
    unsigned int i;

    /* the kind of product made is non-primitive */
    f->process.product_type = 1 + (unsigned int)(rand()%(MAX_PRODUCT_TYPES-1));
    f->process.stock = 0;

    /* note that material inputs can be primitive */
    for (i = 0; i < PROCESS_INPUTS; i++) {
        f->process.raw_material_stock[i] = 0;
        f->process.raw_material[i] = f->process.product_type;
        while (f->process.raw_material[i] == f->process.product_type) {
            f->process.raw_material[i] = (unsigned int)(rand()%MAX_PRODUCT_TYPES);
        }
    }
}

void firm_init(Firm * f)
{
    firm_init_process(f);
    f->labour.wage_rate = MIN_WAGE +
        ((rand()%10000/10000.0f)*(MAX_WAGE - MIN_WAGE));
    f->labour.productivity = MIN_PRODUCTIVITY +
        ((rand()%10000/10000.0f)*(MAX_PRODUCTIVITY - MIN_PRODUCTIVITY));
    f->labour.workers = INITIAL_WORKERS;
    f->labour.days_per_week =
        (unsigned int)(MIN_DAYS_PER_WEEK +
                       ((rand()%10000/10000.0f)*
                        (MAX_DAYS_PER_WEEK - MIN_DAYS_PER_WEEK)));
    f->labour.time_total =
        MIN_WORKING_DAY +
        ((rand()%10000/10000.0f)*(MAX_WORKING_DAY - MIN_WORKING_DAY));
    f->labour.time_necessary = f->labour.time_total/2;
    f->capital.variable = 0;
    f->capital.constant = 10;
    f->capital.surplus = 0;
    f->sale_value = 1.50f;
}

void econ_init(Economy * e)
{
    unsigned int i;

    e->size = MAX_ECONOMY_SIZE/4;
    e->bankruptcies = 0;
    for (i = 0; i < e->size; i++) {
        firm_init(&e->firm[i]);
    }
}

/* See http://www.cybaea.net/Blogs/employee_productivity.html */
float firm_productivity_per_worker(Firm * f)
{
    return f->labour.productivity * INITIAL_WORKERS / (1 + (float)(f->labour.workers));
}

/* fixed outgoings per day. This is assumed to depend on the number of workers */
float firm_constant_per_day(Firm * f)
{
    return f->capital.constant * f->labour.workers;
}

/* total workers wages per day */
float firm_variable_labour_per_day(Firm * f)
{
    return f->labour.wage_rate * f->labour.time_total * f->labour.workers;
}

/* how many products are made per day? */
float firm_products_made_per_day(Firm * f)
{
    return firm_productivity_per_worker(f) * f->labour.time_total * f->labour.workers;
}

float firm_sales_income_per_day(Firm * f, float product_sale_value)
{
    return product_sale_value * firm_products_made_per_day(f);
}

/* given a certain expected profit how much can individual products be sold for? */
float firm_product_sale_value(Firm * f, float surplus_per_day)
{
    return (surplus_per_day + firm_variable_labour_per_day(f) +
            firm_constant_per_day(f)) /
        firm_products_made_per_day(f);
}

/* labour time needed for zero profit */
float firm_necessary_labour_time(Firm * f)
{
    return firm_variable_labour_per_day(f) * f->labour.time_total /
        (firm_sales_income_per_day(f,f->sale_value) - firm_constant_per_day(f));
}

float firm_necessary_variable_labour_per_day(Firm * f)
{
    return f->labour.wage_rate * firm_necessary_labour_time(f) * f->labour.workers;
}

float firm_surplus_per_day(Firm * f)
{
    return firm_sales_income_per_day(f,f->sale_value) -
        (firm_variable_labour_per_day(f) + firm_constant_per_day(f));
}

float firm_worth(Firm * f)
{
    return f->capital.surplus + firm_variable_labour_per_day(f) + firm_constant_per_day(f);
}

void firm_purchasing(Firm * f)
{
    /* TODO */
}

void firm_update(Firm * f, unsigned int weeks)
{
    f->capital.surplus += firm_surplus_per_day(f) * f->labour.days_per_week * weeks;
}

void econ_startups(Economy * e)
{
    unsigned int i;
    Firm * f;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (f->labour.workers == 0) {
            firm_init(f);
        }
    }
    e->bankruptcies=0;
}

void econ_mergers(Economy * e)
{
    unsigned int i, j;
    int best_index;
    Firm * f, * f2;
    float best;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (f->labour.workers == 0) continue;
        best_index = -1;
        best = 0;
        for (j = 0; j < e->size; j++) {
            if (i == j) continue;
            f2 = &e->firm[j];
            if (f2->labour.workers == 0) continue;
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
        if (f->capital.surplus < 0) {
            e->unemployed += f->labour.workers;
            f->labour.workers = 0;
            e->bankruptcies++;
        }
    }
}

void econ_labour_market(Economy * e)
{
    unsigned int i, j;
    int best;
    Firm * f, * f2;
    float max_wage;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (f->labour.workers == 0) continue;
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
        }
    }   
}

void econ_update(Economy * e)
{
    unsigned int i;
    Firm * f;

    econ_startups(e);
    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        firm_update(f, 1);
    }
    econ_bankrupt(e);
    econ_mergers(e);
    econ_labour_market(e);
}

int main(int argc, char* argv[])
{
    Economy e;
    econ_init(&e);
    econ_update(&e);
    printf("Profit: %.2f\n",e.firm[0].capital.surplus);
    printf("Necessary: %.2f\n",firm_necessary_labour_time(&e.firm[0]));
    return 0;
}
