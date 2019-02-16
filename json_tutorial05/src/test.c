//
// Created by SCH on 2019/1/19.
//
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>
#include "leptjson.h"
static int test_count = 0;
static int test_count_parsed = 0;

static void test_parse_access_string();
#define _WINDOWS
#define _CRTDBG_MAP_ALLOC
#define EXPECT_EQ_BASE(equality, expect, actual, object_format) \
        do{\
            test_count++;\
            if(equality){ \
                test_count_parsed++;\
            } \
            else \
                fprintf(stderr, "%s:%d: expect: " object_format ",actual: " object_format "\n", __FILE__, __LINE__, expect, actual);\
            }\
            while(0)

//unit test core
#define EXPECT_EQ_actual(expect, actual, object_format)  EXPECT_EQ_BASE((expect) == (actual), expect, actual, object_format)
#define LEPT_TYPE_INIT(v) do{(v)->leptjson_type = LEPT_NULL;}while(0)
#define EXPECT_EQ_STRING(expect, actual, length) \
                EXPECT_EQ_BASE(sizeof(expect)-1 == length && memcmp(expect, actual, length) == 0, expect, actual, "%s")
#define TEST_STRING(expect, actual) \
                do{ \
                    LeptJsonResult leptJsonResult; \
                    LEPT_TYPE_INIT(&leptJsonResult); \
                    EXPECT_EQ_actual(LEPT_PARSE_OK,LeptJson_Parse(&leptJsonResult, actual), "%d"); \
                    EXPECT_EQ_actual(LEPT_STRING, GetParseResult(&leptJsonResult), "%d"); \
                    EXPECT_EQ_STRING(expect, lept_get_string(&leptJsonResult), lept_get_strlen(&leptJsonResult)); \
                    lept_free(&leptJsonResult); \
                 } while(0)

static void test_parse_character(LeptJsonType leptJsonType, char* string, RetType retType){
        LeptJsonResult leptjson;
        leptjson.leptjson_type = LEPT_NULL;
    EXPECT_EQ_actual(retType, LeptJson_Parse(&leptjson, string), "%d");
    EXPECT_EQ_actual(leptJsonType, GetParseResult(&leptjson), "%d");
    //lept_free(&leptjson);
}
static void test_parse_error(LeptJsonType leptJsonType, char* string, RetType retType) {
    LeptJsonResult leptjson;
    leptjson.leptjson_type = LEPT_NULL;
    EXPECT_EQ_actual(retType, LeptJson_Parse(&leptjson, string), "%d");
    EXPECT_EQ_actual(leptJsonType, GetParseResult(&leptjson), "%d");
    //lept_free(&leptjson);
}
static void test_parse_number(LeptJsonType leptJsonType, char* string, RetType retType){
    LeptJsonResult leptjson;
    leptjson.leptjson_type = LEPT_NULL;
    EXPECT_EQ_actual(retType, LeptJson_Parse(&leptjson, string) ,"%d");
    EXPECT_EQ_actual(leptJsonType, GetParseResult(&leptjson), "%d");
    //lept_free(&leptjson);
}

static void test_parse_string(){
            TEST_STRING("hello", "\"hello\"");
            TEST_STRING("world", "\"world\"");
            TEST_STRING("sfs", "\"sfs\"");
            TEST_STRING("schsch", "\"schsch\"");
            TEST_STRING("\" \\ \r \b \f \n \t / ", "\"\\\" \\\\ \\r \\b \\f \\n \\t \\/ \"");
            TEST_STRING("\0", "\"\\u0000\"");
            TEST_STRING("\x24", "\"\\u0024\"");
            TEST_STRING("\xC2\xA2", "\"\\u00A2\"");  //\x 告诉编译器后面是16进制数
            TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"");
            TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");
            TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");
}
static  void test_parse_arrary(){
            LeptJsonResult leptJsonResult;
            LEPT_TYPE_INIT(&leptJsonResult);
            EXPECT_EQ_actual(LEPT_PARSE_OK, LeptJson_Parse(&leptJsonResult, "[ null , 12,\"adf\" ]"), "%d");
            EXPECT_EQ_actual(LEPT_ARRAY, GetParseResult(&leptJsonResult) ,"%d");
            EXPECT_EQ_actual(3, lept_get_arrary_size(&leptJsonResult), "%d");
            //EXPECT_EQ_actual(1, lept_get_arrary_by_index(&leptJsonResult, 0), "%d");
        }
static void test_parse_access_string() {
    LeptJsonResult leptJsonResult;
    LEPT_TYPE_INIT(&leptJsonResult);
    lept_set_string(&leptJsonResult, "hello", 5);
    EXPECT_EQ_STRING("hello",lept_get_string(&leptJsonResult), lept_get_strlen(&leptJsonResult));

    lept_set_string(&leptJsonResult, "world", 5);
    EXPECT_EQ_STRING("world",lept_get_string(&leptJsonResult), lept_get_strlen(&leptJsonResult));

    lept_set_string(&leptJsonResult, "", 0);
    EXPECT_EQ_STRING("",lept_get_string(&leptJsonResult), lept_get_strlen(&leptJsonResult));
}
static void test_parse_access_boolen(){
            LeptJsonResult leptJsonResult;
            LEPT_TYPE_INIT(&leptJsonResult);
            lept_set_boolen(&leptJsonResult, 0);
            EXPECT_EQ_actual(LEPT_FALSE, lept_get_boolen(&leptJsonResult), "%d");
            lept_set_boolen(&leptJsonResult, 1);
            EXPECT_EQ_actual(LEPT_TRUE, lept_get_boolen(&leptJsonResult), "%d");
            }
static void test_parse_access_number(){
            LeptJsonResult leptJsonResult;
            LEPT_TYPE_INIT(&leptJsonResult);
            lept_set_number(&leptJsonResult, 123341);
            EXPECT_EQ_actual(123341, lept_get_number(&leptJsonResult), "%g");
            lept_set_number(&leptJsonResult, 45e3);
            EXPECT_EQ_actual(45e3, lept_get_number(&leptJsonResult), "%g");
            lept_set_number(&leptJsonResult, 879966);
            EXPECT_EQ_actual(879966, lept_get_number(&leptJsonResult), "%g");
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
    test_parse_number(LEPT_INVALID, "00001", LETP_PARSE_TYPE_NOT_SINGULAR);
    test_parse_number(LEPT_INVALID, "-9e75942379", LEPT_PARSE_NUMBER_TOO_BIG);
    test_parse_error(LEPT_INVALID, "217 asd", LETP_PARSE_TYPE_NOT_SINGULAR);
    test_parse_error(LEPT_INVALID, "adfs", LEPT_PARSE_INVALID_VALUE);
    test_parse_error(LEPT_INVALID, "nulll", LETP_PARSE_TYPE_NOT_SINGULAR);
    test_parse_error(LEPT_INVALID, "false     6", LETP_PARSE_TYPE_NOT_SINGULAR);
    test_parse_error(LEPT_INVALID, " ", LEPT_PARSE_NO_VALUE);
    test_parse_string();
    test_parse_access_string();
    test_parse_access_boolen();
    test_parse_access_number();
    test_parse_arrary();
}


int main() {
#ifdef _WINDOWS
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    TestPareseFuc();
    printf("test count:%d, test pass count:%d, pass rate: %3.2f%%\n", test_count/2, test_count_parsed/2, test_count_parsed*100.0/test_count);
    return 0;
}