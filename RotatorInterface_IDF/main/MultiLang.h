#pragma once

#define NUM_OF_LANGS 2

typedef enum
{
    LANG_ZH_CN = 0,
    LANG_EN_US
} multi_lang_t;

#define DECLATE_GLOBAL_MULTI_LANG_STR(sname) extern const char* sname[]
#define DEFINE_MULTI_LANG_STR_1(sname, sLang0) const char* sname[NUM_OF_LANGS] = {sLang0}
#define DEFINE_MULTI_LANG_STR_2(sname, sLang0, sLang1) const char* sname[NUM_OF_LANGS] = {sLang0, sLang1}


const char* MLSTR(const char** sname, multi_lang_t iLang);