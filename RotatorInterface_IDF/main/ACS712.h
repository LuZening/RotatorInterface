typedef struct
{
    bool isBiDir;
    int nMax_mA;
    int mV_per_amp;
    int mV_zero; 
} ACS712_t;
void ACS712_init(ACS712_t* p, int nMax_mA, int mV_per_amp);
void ACS712_set_zero(ACS712_t *p, int mV_zero);
int ACS712_get_mA_from_mV_value(ACS712_t*p, int mV);