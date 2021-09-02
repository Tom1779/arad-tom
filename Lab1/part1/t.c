#include <stdio.h>
#include <stdarg.h>

typedef unsigned int u32;

char *tab = "0123456789ABCDEF";
int BASE;

int rpu(u32 x)
{
    char c;
    if (x)
    {
        c = tab[x % BASE];
        rpu(x / BASE);
        putchar(c);
    }
}

int printu(u32 x)
{
    (x == 0) ? putchar('0') : rpu(x);
}

// 1-1
void prints(char *s) //print string
{
    if (!s) // exit if no string
    {
        return;
    }

    while (*s != '\0') //iterating until end of string
    {
        putchar(*s); //printing char
        s++;
    }
}

// 1-2
int printd(int x)
{
    BASE = 10; // base 10 for numbers
    if (x < 0) //printing a minus if x is negative
    {
        putchar('-');
        x = -x;
    }
    printu(x); //print int
}

int printx(u32 x)
{
    BASE = 16; // base 16 for hex
    putchar('0');
    putchar('x');
    printu(x); // print hex number
}

int printo(u32 x)
{
    BASE = 8; // base 8 for octal
    putchar('0');
    printu(x); //print octal
}

// 1-3

void myprintf(char *fmt, ...)
{
    int state = 0; // 0 = normal state, 1 = escape

    va_list valist;

    va_start(valist, fmt); // allocating space foran amount of arguements equal to string length

    if (!fmt) // exit if no string
    {
        return;
    }
    while (*fmt != '\0') //iterating while not at the end of the string
    {
        if (!state)
        {
            if (*fmt == '%')
            {
                state = 1;
            }
            else
            {
                putchar(*fmt);
            }
        }
        else // escape state
        {
            if (*fmt == 'c')
            {
                char val = va_arg(valist, int);
                putchar(val);
            }
            else if (*fmt == 's')
            {
                char *val = va_arg(valist, char *);
                prints(val);
            }
            else if (*fmt == 'u')
            {
                u32 val = va_arg(valist, u32);
                printu(val);
            }
            else if (*fmt == 'd')
            {
                int val = va_arg(valist, int);
                printd(val);
            }
            else if (*fmt == 'o')
            {
                u32 val = va_arg(valist, u32);
                printo(val);
            }
            else if (*fmt == 'x')
            {
                u32 val = va_arg(valist, u32);
                printx(val);
            }
            else
            {
                putchar('%');
                putchar(*fmt);
            }
            state = 0;
        }
        fmt++;
    }
    if (state == 1)
    {
        putchar('%');
    }
}

int main(int argc, char *argv[], char *env[])
{
    prints("Tom");
    putchar('\n');
    printd(324);
    putchar('\n');
    printd(-53);
    putchar('\n');
    printx(107);
    putchar('\n');
    printo(8);
    putchar('\n');
    myprintf("cha=%c string=%s      dec=%d hex=%x oct=%o neg=%d\n",
             'A', "this is a test", 100, 100, 100, -100);
    putchar('\n');
    myprintf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        myprintf("argv[%d] = %s\n", i, argv[i]);
    }
    int i = 0;
    while (*env != NULL)
    {
        myprintf("env[%d]: %s\n", i, *env);
        env++;
        i++;
    }

    return 0;
}
