#include "stak.h"

float notes[128];

void
init_notes()
{
    int i;
    for(i = 0; i < 128; ++i)
        notes[i] = (pow(2, (i - 69.0) / 12.0) * 440.0);
}

float
midi2freq(int m)
{
    if(0 <= m && m < 128)
        return notes[m];

    return 0;
}

int
freq2midi(float freq)
{
    return 69 + 12 * log(freq / 440) / log(2);
}
