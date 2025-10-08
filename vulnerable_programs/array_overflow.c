#include <stdio.h>

void arrayOverflow()
{
    int arr[10];

    // Writing beyond array bounds
    for (int i = 0; i <= 10; i++)
    { // Off-by-one error: should be i < 10
        arr[i] = i * 2;
    }

    // Direct out-of-bounds access
    arr[15] = 100;

    // Reading beyond array bounds
    printf("Value: %d\n", arr[20]);
}

void dynamicArrayOverflow()
{
    int size = 5;
    int buffer[size];

    // Loop goes beyond allocated size
    for (int i = 0; i < 10; i++)
    {
        buffer[i] = i;
    }
}

void userControlledIndex(int index)
{
    int data[8];

    // No bounds checking on user input
    data[index] = 42;
}

int main()
{
    arrayOverflow();
    dynamicArrayOverflow();
    userControlledIndex(100); // Attacker can control this
    return 0;
}