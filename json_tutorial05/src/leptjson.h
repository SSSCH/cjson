//
// Created by SCH on 2019/1/19.
//

#include <stdint.h>

#ifndef JSON_TUTORIAL01_LEPTJSON_H
#define JSON_TUTORIAL01_LEPTJSON_H


//待解析的json类型
typedef enum{
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_INVALID
}LeptJsonType;
typedef struct LeptJsonResult LeptJsonResult;  //predelcare前向聲明
//Josn 结果的容器
struct LeptJsonResult{
    union{
        double number;
        struct{
            char* string;
            size_t len;
        }s;  //string
        struct{
            LeptJsonResult *elemts;
            size_t size;   //size_t大小更随系统，32位的是4字节，64位则为8字节
        }a; //arrary
    };
    LeptJsonType leptjson_type;  //枚举一般四个字节
};
//解析完成后返回值类型
typedef enum{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_NO_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LETP_PARSE_TYPE_NOT_SINGULAR,
    LEPT_PARSE_TYPE_ILLEGAL_NUMBER,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOATION_MARK,
    LEPT_PARSE_TYPE_ILLEGAL_STRING,
    LEPT_PARSE_TYPE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_TYPE_INVALID_UNICODE,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
}RetType;
//解析器
int LeptJson_Parse(LeptJsonResult *result, const char *InputJson);

//获取解析结果
int GetParseResult(const LeptJsonResult *result);
#endif //JSON_TUTORIAL01_LEPTJSON_H
//动态获取内存，设置字符串值
void lept_set_string(LeptJsonResult* leptJsonResult, const char* json, size_t len);
//内存初始化
void lept_free(LeptJsonResult* leptJsonResult);
char* lept_get_string(LeptJsonResult* leptJsonResult);
size_t lept_get_strlen(LeptJsonResult *leptJsonResult);
void lept_set_number(LeptJsonResult *leptJsonResult, double number);
void lept_set_boolen(LeptJsonResult* leptJsonResult, int boolen);
double lept_get_number(LeptJsonResult* leptJsonResult);
LeptJsonType lept_get_boolen(LeptJsonResult* leptJsonResult);