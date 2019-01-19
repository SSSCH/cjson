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


static int lept_parse_null(LeptJsonResult *result,  intput_json *intputJson){
    EXPECT_EQ(intputJson, 'n');
    if(intputJson->json[0] == 'u' && intputJson->json[1] && 'l' || intputJson->json[2] == 'l'){
        intputJson->json +=3;
        result->leptjson_type = LEPT_NULL;
        return LEPT_PARSE_OK;
    } else
        return LEPT_PARSE_INVALID_VALUE;
}
static int lept_parse_true(LeptJsonResult *result,  intput_json *intputJson){
    EXPECT_EQ(intputJson, 't');
    if(intputJson->json[0] == 'r' && intputJson->json[1] && 'u' || intputJson->json[2] == 'e'){
        intputJson->json +=3;
        result->leptjson_type = LEPT_TRUE;
        return LEPT_PARSE_OK;
    } else
        return LEPT_PARSE_INVALID_VALUE;
}
static int lept_parse_false(LeptJsonResult *result,  intput_json *intputJson){
    EXPECT_EQ(intputJson, 'f');
    if(intputJson->json[0] == 'a' && intputJson->json[1] && 'l' || intputJson->json[2] == 's' || intputJson->json[3] == 'e'){
        intputJson->json +=4;
        result->leptjson_type = LEPT_FALSE;
        return LEPT_PARSE_OK;
    } else
        return LEPT_PARSE_INVALID_VALUE;
}
static int lept_parse_number(LeptJsonResult *result, intput_json *intputJson){
    char * end;
    result->number = strtod(intputJson->json, &end);
    if(intputJson->json == end){//strtod 会在遇到非数字后将其后第一个字节的地址传入end，intputJson->josn == end说明一上来就不是数字
        result->leptjson_type = LEPT_INVALID;
        return LEPT_PARSE_INVALID_VALUE;
    }
    else{
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
            return lept_parse_null(result, intputJson);
        case 't' :
            return lept_parse_true(result, intputJson);
        case 'f' :
            return lept_parse_false(result, intputJson);
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