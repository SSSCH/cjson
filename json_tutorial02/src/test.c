//
// Created by SCH on 2019/1/19.
//
#include <stdlib.h>
#include <stdio.h>
#include "leptjson.h"
static int test_count = 0;
static int test_count_parsed = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, object_format) \
        do{\
            test_count++;\
            if(equality)\
                test_count_parsed++;\
            else\
                fprintf(stderr, "%s:%d: expect: " object_format ",actual: " object_format "\n", __FILE__, __LINE__, expect, actual);\
            }\
            while(0)

//unit test core
#define EXPECT_EQ_actual(expect, actual, object_format)  EXPECT_EQ_BASE((expect) == (actual), expect, actual, object_format)

static void test_parse_character(LeptJsonType leptJsonType, char* string, RetType retType){
        LeptJsonResult leptjson;
        leptjson.leptjson_type = LEPT_NULL;
    EXPECT_EQ_actual(retType, LeptJson_Parse(&leptjson, string), "%d");
    EXPECT_EQ_actual(leptJsonType, GetParseResult(&leptjson), "%d");
}
static void test_parse_error(LeptJsonType leptJsonType, char* string, RetType retType) {
    LeptJsonResult leptjson;
    leptjson.leptjson_type = LEPT_NULL;
    EXPECT_EQ_actual(retType, LeptJson_Parse(&leptjson, string), "%d");
    EXPECT_EQ_actual(leptJsonType, GetParseResult(&leptjson), "%d");
}
static void test_parse_number(LeptJsonType leptJsonType, char* string, RetType retType){
    LeptJsonResult leptjson;
    leptjson.leptjson_type = LEPT_NULL;
    EXPECT_EQ_actual(retType, LeptJson_Parse(&leptjson, string) ,"%d");
    EXPECT_EQ_actual(leptJsonType, GetParseResult(&leptjson), "%d");
}

static void TestPareseFuc(){
    test_parse_character(LEPT_NULL, "null", LEPT_PARSE_OK);
    test_parse_character(LEPT_FALSE, "false", LEPT_PARSE_OK);
    test_parse_character(LEPT_TRUE, "true", LEPT_PARSE_OK);
    test_parse_character(LEPT_TRUE, "true   ", LEPT_PARSE_OK);
    test_parse_number(LEPT_NUMBER, "1231", LEPT_PARSE_OK);
    test_parse_number(LEPT_NUMBER, "243", LEPT_PARSE_OK);
    test_parse_number(LEPT_NUMBER, "354.6", LEPT_PARSE_OK);
    test_parse_number(LEPT_NUMBER, "3423.6", LEPT_PARSE_OK);
    test_parse_number(LEPT_NUMBER, "-41234", LEPT_PARSE_OK);
    test_parse_number(LEPT_NUMBER, "21342", LEPT_PARSE_OK);
    test_parse_number(LEPT_NUMBER, "2e+2", LEPT_PARSE_OK);
    test_parse_error(LEPT_INVALID, "217 asd", LETP_PARSE_TYPE_NOT_SINGULAR);
    test_parse_error(LEPT_INVALID, "adfs", LEPT_PARSE_INVALID_VALUE);
    test_parse_error(LEPT_INVALID, "nulll", LETP_PARSE_TYPE_NOT_SINGULAR);
    test_parse_error(LEPT_INVALID, "false     6", LETP_PARSE_TYPE_NOT_SINGULAR);
    test_parse_error(LEPT_INVALID, " ", LEPT_PARSE_NO_VALUE);

}

int main() {
    TestPareseFuc();
    printf("test count:%d, test pass count:%d, pass rate: %3.2f%%\n", test_count/2, test_count_parsed/2, test_count_parsed*100.0/test_count);
    return 0;
}