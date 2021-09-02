#include <stdio.h>

typedef unsigned int u32;

char *tab = "0123456789ABCDEF";
int  BASE = 10; 

int rpu(u32 x)
{  
    char c;
    if (x){
       c = tab[x % BASE];
       rpu(x / BASE);
       putchar(c);
    }
}

int printu(u32 x)
{
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}

// 1-1
void prints(char* s)
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

int main()
{
    prints("Tom");
    putchar('\n');

    return 0;
}



