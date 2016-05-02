/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Sobey.
 *       Filename:  main.c
 *
 *    Description:  
 *         Others:
 *
 *        Version:  1.0
 *        Created:  04/29/2016 02:47:18 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy. Hou (hwt), houwentaoff@gmail.com
 *   Organization:  Jz
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kfifo.h"

#define MAX_LINE      48            /*  */

kfifo_t test;
static char buf[2048];//[MAX_LINE][MAX_LINE];

char g_str[MAX_LINE];

int print_buf(const char *src, unsigned int len)
{
#if 1
    int i=0;
    if (!src)
    {
        return -1;
    }
    printf("\nprt buffer: begin\n");
    for (i=0; i<len; i++)
    {
        if (0 == (i%48))
        {
            printf("\n");
        }
        printf("%c ", src[i]);
    }
    printf("\nprt buffer: end\n");
#endif
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
    memset(&test, 0, sizeof(test));
    memset(&g_str, 0, sizeof(g_str));

    kfifo_init(&test, (void *)buf, sizeof(buf));
    kfifo_in(&test, (void *)g_str, MAX_LINE);
    memcpy((void *)g_str, "A G E N T E S   A U T O R I Z A D O S                              ", MAX_LINE);
    kfifo_in(&test, (void *)g_str, MAX_LINE);
    memcpy((void *)g_str, "E M                                                ", MAX_LINE);
    kfifo_in(&test, (void *)g_str, MAX_LINE);
    memcpy((void *)g_str, "T O D O   O   P A I S                                      ", MAX_LINE);
    kfifo_in(&test, (void *)g_str, MAX_LINE);

    memcpy((void *)g_str, "F A T U R A   S I M P L I F I C A D A                              ", MAX_LINE);
    kfifo_in(&test, (void *)g_str, MAX_LINE);
    memcpy((void *)g_str, "N o . 0 0 2 1 6                                         ", MAX_LINE);
    kfifo_in(&test, (void *)g_str, MAX_LINE);
    print_buf((void *)buf, 2048);    
//    memcpy((void *)g_str, "2 0 1 6 - 0 4 - 2 9   1 4 : 2 5 : 3 0                              ", MAX_LINE);
    memcpy((void *)g_str, "2 0 1 6 - 0 4 - 2 9   1 4 : 2 5 : 3 0  ", MAX_LINE);
    kfifo_in(&test, (void *)g_str, MAX_LINE);
//    memcpy((void *)g_str, "", MAX_LINE)

    print_buf((void *)buf, 2048);

    return EXIT_SUCCESS;
}
