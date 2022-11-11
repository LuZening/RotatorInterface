#include "ACS712.h"

void ACS712_init(ACS712_t* p, int nMax_mA, int mV_per_amp)
{
    p->mV_per_amp = mV_per_amp;
    p->mV_zero = 2500;
    p->nMax_mA = nMax_mA;
}

void ACS712_set_zero(ACS712_t *p, int mV_zero)
{
    p->mV_zero = mV_zero;
}

int ACS712_get_mA_from_mV_value(ACS712_t*p, int mV)
{
    return (mV - p->mV_zero) * 1000 / p->mV_per_amp;
}