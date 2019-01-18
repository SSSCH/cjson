//
// Created by SCH on 2019/1/17.
//

#include <assert.h> //assert()
#include "leptjson.h"
#include <stdlib.h>  //NULL

#define EXPECT(ptr, ch)  do{assert(*ptr->json == (ch));ptr->json++;}while(0)

//为了减少解析函数之间传递多个参数，我们把这些数据都放进同一个结构体
typedef struct {
    const char* json;
}inputJson;

static int lept_parse_whitespace(inputJson *c){
    const char *p = c->json;
    //空格，制表符，换行符， 回车符
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' )
        p++;
    c->json = p;
    assert(c->json!=NULL);
    return 0;
}

static int parse_value_null(type_value *result, inputJson *jsonContext){
    EXPECT(jsonContext, 'n');
    if(jsonContext->json[0] == 'u' && jsonContext->json[1] == 'l' && jsonContext->json[2] == 'l'){
        if(jsonContext->json[3] == '\0'){
            jsonContext->json +=3;
            result->type = LEPT_NULL;
            return LEPT_PARSE_ok;
        }
        else
            return LEPT_PARSE_ROOT_NOOT_SINGULAR;

    }
    else
        return LEPT_PARSE_INVALID_VALUE;
}
static int parse_value_false(type_value *result, inputJson *jsonContext){
    EXPECT(jsonContext, 'f');
    if(jsonContext->json[0] == 'a' && jsonContext->json[1] == 'l' && jsonContext->json[2] == 's' && jsonContext->json[3] == 'e'){
        if(jsonContext->json[4] == '\0'){
            jsonContext->json +=4;
            result->type = LEPT_FALSE;
            return LEPT_PARSE_ok;
        } else
            return LEPT_PARSE_ROOT_NOOT_SINGULAR;
    }
    else
        return LEPT_PARSE_INVALID_VALUE;
}
static int parse_value_true(type_value *result, inputJson *jsonContext){
    EXPECT(jsonContext, 't');
    if(jsonContext->json[0] == 'r' && jsonContext->json[1] == 'u' && jsonContext->json[2] == 'e'){
        if(jsonContext->json[3] == '\0'){
            jsonContext->json +=3;
            result->type = LEPT_TRUE;
            return LEPT_PARSE_ok;
        }
        else
            return LEPT_PARSE_ROOT_NOOT_SINGULAR;
    }
    else
        return LEPT_PARSE_INVALID_VALUE;
}
static int leptJson_parse_value(type_value *result, inputJson *jsonContext){
    switch (*jsonContext->json){
        //null
        case 'n':
            return parse_value_null(result, jsonContext);
        //true
        case 't':
            return parse_value_true(result, jsonContext);
        //false
        case 'f':
            return parse_value_false(result,jsonContext);
        //kong  ' '
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
        //invaild value
        default:
            return LEPT_PARSE_INVALID_VALUE;
    }
}

//解析器
int leptJson_parse(type_value *result, const char* json){
    assert(result != NULL);
    inputJson v;
    v.json = json;
    result->type = LEPT_NULL;
    lept_parse_whitespace(&v);
    return(leptJson_parse_value(result, &v));

};

//获取解析结果
lept_type get_leptJsonType(const type_value * value){
    assert(value != NULL);
    return value->type;
};