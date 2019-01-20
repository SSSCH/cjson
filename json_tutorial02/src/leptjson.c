//
// Created by SCH on 2019/1/19.
//

#include "leptjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define EXPECT_EQ(c, ch)     do{ assert(*c->json == (ch));c->json++; } while(0)
typedef struct {
    char *json;
}intput_json;

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9 ((ch) >= '1' && (ch) <= '9')
static int lept_parse_literal(LeptJsonResult *result, intput_json *inputJson, const char *literal, LeptJsonType josnType ){
    size_t i = 0; //ps，在c语言中数据长度，索引值最好使用“size_t”类型，而不是用int或者unsigned
    EXPECT_EQ(inputJson, literal[0]);
    for(i=0; literal[i+1]; i++){
        if(inputJson->json[i] != literal[i+1]){ //注意：在EXPECT_EQ中，inputJson->json++，所以是从下标1开始判断
            result->leptjson_type = LEPT_PARSE_INVALID_VALUE;
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    inputJson->json += i;
    result->leptjson_type = josnType;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(LeptJsonResult *result, intput_json *intputJson){
    char * end;
    result->number = strtod(intputJson->json, &end);
    if(intputJson->json == end){//strtod 会在遇到非数字后将其后第一个字节的地址传入end，intputJson->josn == end说明一上来就不是数字
        result->leptjson_type = LEPT_INVALID;
        return LEPT_PARSE_INVALID_VALUE;
    }
    else{
        if(intputJson->json[0] == '0' && intputJson->json[1]){ //c语言中字符常量是int型，c++中是char型。所以这边若是写出intputJson->json[0] == 0也不会报错，小心！！！
            result->leptjson_type = LEPT_INVALID;
            return LEPT_PARSE_TYPE_ILLEGAL_NUMBER;
        }
        intputJson->json = end;
        result->leptjson_type = LEPT_NUMBER;
        return LEPT_PARSE_OK;
    }
}
int lept_parse_whitespcae(intput_json *inputjson){
    const char *p = inputjson->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    inputjson->json = p;

}

static int lept_parse_value(LeptJsonResult *result, const intput_json *intputJson){
    switch (*(intputJson->json)){
        case 'n' :
            //return lept_parse_null(result, intputJson);
            return lept_parse_literal(result,intputJson, "null", LEPT_NULL);
        case 't' :
            //return lept_parse_true(result, intputJson);
            return lept_parse_literal(result,intputJson, "true", LEPT_TRUE);
        case 'f' :
            //return lept_parse_false(result, intputJson);
            return lept_parse_literal(result,intputJson, "false", LEPT_FALSE);
        case '\0' :
            return LEPT_PARSE_NO_VALUE;
        default:
            return lept_parse_number(result, intputJson);
    }
}
int LeptJson_Parse(LeptJsonResult *result, const char *InputJson){
    intput_json v;
    int ret;
    assert(result != NULL);
    v.json = InputJson;
    result->leptjson_type = LEPT_INVALID;
    lept_parse_whitespcae(&v);
    if((ret = lept_parse_value(result, &v)) == LEPT_PARSE_OK){
        lept_parse_whitespcae(&v);
        if(*v.json != '\0'){
            result->leptjson_type = LEPT_INVALID;
            ret = LETP_PARSE_TYPE_NOT_SINGULAR;
        }
    }
    return ret;
}

int GetParseResult(const LeptJsonResult *result){
    assert(result != NULL);
    printf("parse leptjosn result:\n");
    printf("leptjosn type:%d\n", result->leptjson_type);
    if(result->leptjson_type == LEPT_NUMBER)
        printf("number:%.17g.\n", result->number);
    return result->leptjson_type;
}