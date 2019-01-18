#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int test_count = 0;
static int main_ret = 0;
static int test_pass_count = 0;
//判断预期值和实际值是否相等
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
        do{\
            test_count++;\
            if(equality)\
                test_pass_count++;\
                else{\
                    fprintf(stderr,"%s:%d:expect: " format " actual:" format "\n", __FILE__, __LINE__, expect, actual);\
                    main_ret = 1;\
                }\
                }while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect)==(actual), expect, actual, "%d")

static void test_parse_null(){
    type_value v;
    v.type = LEPT_NULL;
    EXPECT_EQ_INT(LEPT_PARSE_ok, leptJson_parse(&v, "null     "));
    EXPECT_EQ_INT(LEPT_NULL,get_leptJsonType(&v));
}
static void test_parse_false(){
    type_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_ok, leptJson_parse(&v, "false  "));
    EXPECT_EQ_INT(LEPT_FALSE,get_leptJsonType(&v));
}
static void test_parse_ture(){
    type_value v;
    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_ok, leptJson_parse(&v, "true  "));
    EXPECT_EQ_INT(LEPT_TRUE,get_leptJsonType(&v));
}
static void test_parse_expect_value(){
    type_value v;
    v.type = LEPT_NULL;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, leptJson_parse(&v, " "));
    EXPECT_EQ_INT(LEPT_NULL,get_leptJsonType(&v));
}
static void test_parse_invalid_value(){
    type_value v;
    v.type = LEPT_NULL;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, leptJson_parse(&v, "nul"));
    EXPECT_EQ_INT(LEPT_NULL,get_leptJsonType(&v));
}
static void test_pars_root_noot_singular(){
    type_value v;
    v.type = LEPT_NULL;
    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOOT_SINGULAR, leptJson_parse(&v, "false 9"));
    EXPECT_EQ_INT(LEPT_NULL,get_leptJsonType(&v));
}

static void test_parse() {
    test_parse_null();
    test_parse_false();
    test_parse_ture();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_pars_root_noot_singular();
}
int main() {
    test_parse();
    printf("testcount:%d,parsecount:%d,pass rate:%3.2f%%\n", test_count, test_pass_count,test_pass_count*100.0/test_count);
    return 0;
}