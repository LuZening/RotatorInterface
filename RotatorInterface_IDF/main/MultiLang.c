#include "MultiLang.h"

#include <stdint.h>
const char* MLSTR(const char** sname, multi_lang_t iLang)
{
    uint16_t i = (uint16_t)iLang;
    if (iLang < NUM_OF_LANGS)
    {
        return sname[iLang];
    }
    else
    {
        return sname[0];
    }
}