#include "entry.h"

int gcd(int a, int b)
{
    if (a == 0)
        return b;
    if (b == 0)
        return a;

    if (a == b)
        return a;

    if (a > b)
        return gcd(a - b, b);
    else
        return gcd(a, b - a);
}


int main(void)
{
    int a = 134;
    int b = 32;

    tohost = gcd(a, b);
    return 0;
}
