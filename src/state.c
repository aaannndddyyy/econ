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
	s->capital.surplus = INITIAL_STATE_DEPOSIT;
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
		((rand()%10000/10000.0)*(MAX_BUSINESS_TAX_RATE - MIN_BUSINESS_TAX_RATE));
}
