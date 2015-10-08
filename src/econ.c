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
    Firm * f;

    e->size = MAX_ECONOMY_SIZE/4;
    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        f->labour.wage_rate = 1;
        f->labour.productivity=1;
        f->labour.workers = 20;
        f->labour.time = 8;
        f->capital.variable = 0;
        f->capital.constant = 1;
        f->capital.surplus = 0;
        f->production = 0;
    }
}

void firm_update(Firm * f)
{
    float products_made = f->labour.productivity * f->labour.time * f->labour.workers;
    f->production = f->sale_value * products_made;
    f->capital.variable = f->labour.wage_rate * f->labour.time * f->labour.workers;
    f->capital.surplus = f->production - (f->capital.variable + (f->capital.constant*f->labour.workers));
}

void econ_update(Economy * e)
{
    unsigned int i;
    Firm * f;

    for (i = 0; i < e->size; i++) {
        f = &e->firm[i];
        firm_update(f);
    }
}

int main(int argc, char* argv[])
{
    Economy e;
    econ_init(&e);
    return 0;
}
