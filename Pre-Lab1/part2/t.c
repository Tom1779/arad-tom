/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

int A(int x, int y);
int B(int x, int y);
int C(int x, int y);
void *getebp();

int main(int argc, char *argv[], char *env[])
{
    int a, b, c;
    printf("enter main\n");

    printf("&argc=%p argv=%p env=%p\n", &argc, argv, env);
    printf("&a=%p &b=%p &c=%p\n", &a, &b, &c);

    //(1).Write C code to print values of argc and argv[] entries
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    a = 1;
    b = 2;
    c = 3;
    A(a, b);
    printf("exit main\n");
}

int A(int x, int y)
{
    int d, e, f;
    printf("enter A\n");
    // Printing x and y addresses
    printf("&x=%p &y=%p\n", &x, &y);

    // write C code to PRINT ADDRESS OF d, e, f
    printf("&d=%p &e=%p &f=%p\n", &d, &e, &f);
    d = 4;
    e = 5;
    f = 6;
    B(d, e);
    printf("exit A\n");
}

int B(int x, int y)
{
    int g, h, i;
    printf("enter B\n");
    // Printing x and y addresses
    printf("&x=%p &y=%p\n", &x, &y);

    // write C code to PRINT ADDRESS OF g,h,i
    printf("&g=%p &h=%p &i=%p\n", &g, &h, &i);
    g = 7;
    h = 8;
    i = 9;
    C(g, h);
    printf("exit B\n");
}

int C(int x, int y)
{
    int u, v, w, i, *p;

    printf("enter C\n");
    // Printing x and y addresses
    printf("&x=%p &y=%p\n", &x, &y);

    // write C cdoe to PRINT ADDRESS OF u,v,w,i,p;
    printf("&u=%p &v=%p &w=%p &i=%p &p=%p\n", &u, &v, &w, &i, &p);
    u = 10;
    v = 11;
    w = 12;
    i = 13;

    FP = (int *)getebp(); // FP = stack frame pointer of the C() function

    //print FP value in HEX
    printf("FP=%p\n", FP);

    //(2).Write C code to print the stack frame link list.
    /**
     * Func main 
     *   ebp->  saved ebp = 0
     *          A/y
     *          A/x
     *          local variables
     *          return address to main
     * Func A
     *   ebp->  main/ebp
     *          B/y
     *          B/x
     *          return address to A
     * Func B
     *   ebp->  A/ebp
     *          C/y
     *          C/x
     *          return address to B
     * Func C
     *   ebp->  B/ebp
     */


    p = FP;
    while (p != NULL)
    {
        printf("FP: %p\n", p);
        p = (int *)*p;
    }

    //p = (int *)&p;

    /*(3).Print the stack contents from p to the frame of main()
        YOU MAY JUST PRINT 128 entries of the stack contents.*/
    p = FP;
    for(int i = 0; i < 128; i++)
    {
        printf("%3d. [%p] %8x\n", i, p, *p);
        p--;
    }


    /*  (4). On a hard copy of the print out,
        identify the stack contents as LOCAL VARIABLES,
        PARAMETERS, stack frame pointer of each function.*/

        
}
