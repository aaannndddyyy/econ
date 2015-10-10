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
    f->location = (unsigned int)(rand()%LOCATIONS);
    f->labour.wage_rate = MIN_WAGE +
        ((rand()%10000/10000.0f)*(MAX_WAGE - MIN_WAGE));
    f->labour.productivity = MIN_PRODUCTIVITY +
        ((rand()%10000/10000.0f)*(MAX_PRODUCTIVITY - MIN_PRODUCTIVITY));
    f->labour.workers = INITIAL_WORKERS;
    f->labour.is_recruiting = 0;
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
    f->capital.surplus = INITIAL_CREDIT;
    f->sale_value = 1.50f;
}

int firm_defunct(Firm * f)
{
    return (f->labour.workers == 0);
}

void merchant_init(Merchant * m)
{
    unsigned int i;

    m->capital.variable = 0;
    m->capital.constant = 10;
    m->capital.surplus = INITIAL_MERCHANT_CREDIT;
    m->interest_rate = 2;
    m->hedge = MAX_PRODUCT_TYPES/2;
    for (i = 0; i < MAX_PRODUCT_TYPES; i++) {
        m->stock[i] = 0;
        m->price[i] = 0;
    }
}

void bank_init(Bank * b)
{
    unsigned int i;

    b->capital.variable = 0;
    b->capital.constant = 0;
    b->capital.surplus = INITIAL_BANK_CREDIT;
    b->interest_credit = 0.5f;
    b->interest_debt = 10;
    for (i = 0; i < MAX_ECONOMY_SIZE+1; i++) {
        b->balance[i] = 0;
    }
}

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

float firm_products_which_can_be_made(Firm * f)
{
    unsigned int i;
    float new_products=0;

    /* how many new products can be made with the raw materials? */
    for (i = 0; i < PROCESS_INPUTS; i++) {
        if ((new_products==0) || (f->process.raw_material_stock[i] < new_products)) {
            new_products = f->process.raw_material_stock[i];
        }
    }
    return new_products;
}

/* how many products can be made and sold in one day, given raw materials */
float firm_sales_income_per_day_actual(Firm * f, float product_sale_value)
{
    float max_products_made = firm_products_made_per_day(f);
    float potential_products = firm_products_which_can_be_made(f);
    if (potential_products > max_products_made)
        return product_sale_value * max_products_made;
    return product_sale_value * potential_products;
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

float firm_surplus_per_day_actual(Firm * f)
{
    return firm_sales_income_per_day_actual(f,f->sale_value) -
        (firm_variable_labour_per_day(f) + firm_constant_per_day(f));
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

void firm_strategy(Firm * f, Economy * e)
{
    float possible_surplus, average_price, original_sale_value;
    float existing_surplus = firm_surplus_per_day(f);
    unsigned int workers;

    if (firm_defunct(f)) return;

    /* will recruiting more workers increase surplus ? */
    if (f->labour.workers < MAX_WORKERS) {
        f->labour.is_recruiting = 0;
        f->labour.workers++;
        possible_surplus = firm_surplus_per_day(f);
        f->labour.workers--;
        if (possible_surplus > existing_surplus) {
            f->labour.is_recruiting = 1;
        }
    }

    /* will laying off workers increase surplus ? */
    if ((f->labour.workers > 2) && (f->labour.is_recruiting == 0)) {
        workers = f->labour.workers;
        while ((firm_surplus_per_day(f) < 0) && (f->labour.workers > MIN_WORKERS)) {
            f->labour.workers--;
        }
        if (f->labour.workers != workers) {
            e->unemployed += workers - f->labour.workers;
        }
    }

    /* increase price if we are below the market average */
    average_price = econ_average_price(e, f->process.product_type, f->location);
    if (average_price*0.95f > f->sale_value) {
        f->sale_value *= 1.01f;
    }

    /* if the sale price can be made more competitive without
       making a loss then decrease the sale value */
    if (average_price*1.05f < f->sale_value) {
        original_sale_value = f->sale_value;
        f->sale_value *= 0.99f;
        if (firm_surplus_per_day(f) <= 0) {
            f->sale_value = original_sale_value;
        }
    }
}

float firm_worth(Firm * f)
{
    return f->capital.surplus + firm_variable_labour_per_day(f) + firm_constant_per_day(f);
}

void firm_update(Firm * f, unsigned int weeks)
{
    unsigned int i, days;
    float new_products, products_per_day;

    /* how many days can we go without running out of raw materials ? */
    days = f->labour.days_per_week * weeks;
    new_products = firm_products_which_can_be_made(f);
    products_per_day = firm_products_made_per_day(f);
    if (products_per_day*days < new_products/products_per_day) {
        days = (unsigned int)(new_products / products_per_day);
    }

    f->capital.surplus += firm_surplus_per_day_actual(f) * days;
    f->process.stock += (products_per_day * days);
    for (i = 0; i < PROCESS_INPUTS; i++) {
        f->process.raw_material_stock[i] -= (products_per_day * days);
        if (f->process.raw_material_stock[i] < 0) {
            f->process.raw_material_stock[i] = 0;
        }
    }
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

void firm_buy_raw_material_from_merchant(Firm * f, Economy * e, unsigned int index, float quantity)
{
    unsigned int product_type = f->process.raw_material[index];
    Merchant * m = &e->merchant;
    float buy_qty = quantity;

    if (quantity < 1) return;

    if (m->stock[product_type] == 0) return;
    if (m->stock[product_type] < buy_qty) {
        buy_qty = m->stock[product_type];
    }
    if (buy_qty * m->price[product_type] > f->capital.surplus) {
        buy_qty = f->capital.surplus / m->price[product_type];
    }
    if (buy_qty < 1) return;
    m->stock[product_type] -= buy_qty;
    f->process.raw_material_stock[index] += buy_qty;
    m->capital.surplus += buy_qty * m->price[product_type];
    f->capital.surplus -= buy_qty * m->price[product_type];
    if (f->capital.surplus < 0) f->capital.surplus = 0;
}

void firm_buy_raw_material_locally(Firm * f, Economy * e, unsigned int index, float quantity)
{
    int best_index;
    float quantity_available, buy_quantity;
    unsigned int product_type = f->process.raw_material[index];
    Firm * supplier;

    if (quantity < 1) return;

    best_index = econ_best_price(e, f, product_type, 1);
    while ((best_index > -1) && (f->capital.surplus > 0) && (quantity > 0)) {
        supplier = &e->firm[best_index];
        quantity_available = supplier->process.stock;
        buy_quantity = quantity;
        if (buy_quantity > quantity_available) buy_quantity = quantity_available;
        if (buy_quantity*supplier->sale_value > f->capital.surplus) {
            buy_quantity = f->capital.surplus / supplier->sale_value;
        }

        f->capital.surplus -= supplier->sale_value * buy_quantity;
        if (f->capital.surplus < 0) f->capital.surplus = 0;
        supplier->capital.surplus += supplier->sale_value * buy_quantity;
        f->process.raw_material_stock[index] += buy_quantity;
        supplier->process.stock -= buy_quantity;
        if (supplier->process.stock < 0) supplier->process.stock = 0;
        quantity -= buy_quantity;

        best_index = econ_best_price(e, f, product_type, 1);
    }
}

void firm_purchasing(Firm * f, Economy * e, unsigned int weeks)
{
    unsigned int i;
    float purchases_required;

    for (i = 0; i < PROCESS_INPUTS; i++) {
        purchases_required =
            (firm_products_made_per_day(f) * f->labour.days_per_week * weeks) -
            f->process.raw_material_stock[i];

        if (f->process.raw_material[i] == PRODUCT_PRIMITIVE) {
            f->process.raw_material_stock[i] += purchases_required;
            continue;
        }

        firm_buy_raw_material_from_merchant(f, e, i, purchases_required);

        purchases_required =
            (firm_products_made_per_day(f) * f->labour.days_per_week * weeks) -
            f->process.raw_material_stock[i];
        firm_buy_raw_material_locally(f, e, i, purchases_required);
    }
}

void merchant_buy(Economy * e)
{
    unsigned int i;
    Merchant * m = &e->merchant;
    Firm * f;
    int best_index;
    float investment_tranche = m->capital.surplus / (float)m->hedge;
    float buy_qty, target_price, variance, variance_min=0, variance_max=0;
    float average_variance;

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
                m->capital.surplus -= f->sale_value * buy_qty;
                f->capital.surplus += f->sale_value * buy_qty;
            }
        }
    }
}

void merchant_update(Economy * e)
{
    merchant_buy(e);
}

void econ_update(Economy * e, unsigned int weeks)
{
    unsigned int i;
    Firm * f;

    econ_startups(e);
    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        if (firm_defunct(f)) continue;
        firm_purchasing(f, e, weeks);
        firm_update(f, weeks);
        firm_strategy(f, e);
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
        printf("Necessary: %.2f\n",firm_necessary_labour_time(&e.firm[0]));
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
