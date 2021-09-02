#include <stdio.h>

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
    putchar(' ');
}

// 1-1
void prints(char *s)
{
    if (!s)
    {
        return;
    }

    while (*s != '\0')
    {
        putchar(*s);
        s++;
    }
}

// 1-2
int printd(int x)
{
    BASE = 10;
    if (x < 0)
    {
        putchar('-');
        x = -x;
    }
    printu(x);
}

int printx(u32 x)
{
    BASE = 16;
    putchar('0');
    putchar('x');
    printu(x);
}

int printo(u32 x)
{
    BASE = 8;
    putchar('0');
    printu(x);
}

int main()
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
    
    return 0;
}
