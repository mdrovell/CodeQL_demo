#include <stdio.h>
#include <stdlib.h>

void use_after_free()
{
    int *p = malloc(100);
    free(p);
    *p = 5;
}

// Example program of a common security risk
int main()
{
    use_after_free();
    return 0;
}