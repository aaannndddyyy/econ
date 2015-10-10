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

#ifndef ECON_H
#define ECON_H

#define MAX_ECONOMY_SIZE      1024

#define LABOUR_TIME_TOTAL     0
#define LABOUR_TIME_NECESSARY 1

#define MIN_WORKING_DAY       8
#define MAX_WORKING_DAY       12

#define MIN_DAYS_PER_WEEK     4
#define MAX_DAYS_PER_WEEK     6

#define MIN_WAGE              6.70
#define MAX_WAGE             20.00

#define INITIAL_WORKERS       10
#define MIN_WORKERS           4
#define MAX_WORKERS           2000

#define MIN_PRODUCTIVITY      6
#define MAX_PRODUCTIVITY      50

#define MAX_PRODUCT_TYPES     4
#define PROCESS_INPUTS        2
#define PRODUCT_PRIMITIVE     0

#define INITIAL_CREDIT        10000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
    float variable, constant;
    float surplus;
} Capital;

typedef struct
{
    unsigned int days_per_week;
    float time_total;
    float time_necessary;
    unsigned int workers;
    float wage_rate;
    float productivity;
	unsigned char is_recruiting;
} Labour;

typedef struct
{
    unsigned int raw_material[PROCESS_INPUTS];
    float raw_material_stock[PROCESS_INPUTS];
    unsigned int product_type;
	float stock;
} Process;

typedef struct
{
    Capital capital;
    Labour labour;
    Process process;
    float sale_value;
} Firm;

typedef struct
{
    unsigned int size;
    Firm firm[MAX_ECONOMY_SIZE];
    unsigned int population;
    unsigned int unemployed;
    unsigned int bankruptcies;
} Economy;

#endif
