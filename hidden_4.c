#include <stdio.h>
#include <stdint.h>

#define NUM_PLACES 4
#define NUM_VALUES 6
#define SET_SIZE 1296 // 6^4

void construct_initial_possible_set(unsigned *possibles)
{
    for (unsigned i = 0; i < SET_SIZE; i++)
    {
        possibles[i] = i;
    }
}

int main(void)
{
    unsigned possibles[SET_SIZE];
    return 0;
}
