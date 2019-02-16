//
// Created by SCH on 2019/1/19.
//

#include "leptjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <memory.h>

#define _WINDOWS
#define LEPT_PARSE_STACK_INIT_SIZE    2
#define EXPECT_EQ(c, ch)     do{ assert(*c->json == (ch));c->json++; } while(0)
#define PUTC(p, ch)          do{ *(char*)lept_context_push(p, sizeof(char)) = (ch); } while(0)  //堆栈以字节为单位储存
//解析结果存放的临时缓冲区，采用动态数组的结构，能在空间不足时候自动扩展。即是一个 动态堆栈（stack）结构。
#define TESTED_ERROR_STRING(v, ret) do{v->top = head;return ret;} while(0)
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif


typedef struct {
    const char *json;
    char* stack; //动态数组
    size_t size, top; //@size 当前堆栈容量， @top 栈顶的位置
}intput_json;

void * lept_parse_hex4(const char *p, unsigned int *pInt);

void lept_parse_utf8(intput_json *json, unsigned int u);

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
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

/*static int lept_parse_number(LeptJsonResult *result, intput_json *intputJson){
    errno = 0;
    char * end = NULL;
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
        if(errno == ERANGE && (result->number == HUGE_VAL || result->number == -HUGE_VAL)){
            result->leptjson_type = LEPT_INVALID;
            return LEPT_PARSE_TYPE_NUMBER_TOO_BIG;
        }
        intputJson->json = end;
        result->leptjson_type = LEPT_NUMBER;
        return LEPT_PARSE_OK;
    }
}*/
static int lept_parse_number(LeptJsonResult *v, intput_json *c) {
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->number = strtod(c->json, NULL);
    if (errno == ERANGE && (v->number == HUGE_VAL || v->number == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->leptjson_type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}
int lept_parse_whitespcae(intput_json *inputjson){
    const char *p = inputjson->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    inputjson->json = p;

}

//作用：检查堆栈大小，根据push的size移动top指针，并且返回当前栈顶指针
static void* lept_context_push(intput_json* intputJson, size_t size){
    void* ret;
    assert(size > 0);
    if(intputJson->size <= intputJson->top + size){
        if(intputJson->size == 0){
            intputJson->size = LEPT_PARSE_STACK_INIT_SIZE;
        }
        while(intputJson->size <= intputJson->top + size){
            intputJson->size += intputJson->size >> 1; //size*1.5
            intputJson->stack = (char*)realloc(intputJson->stack, intputJson->size);
        }
    }
    ret = intputJson->stack + intputJson->top;
    intputJson->top += size;
    return ret;
}

//返回字符串首地址指针
static void* lept_context_pop(intput_json* intputJson, size_t len){
    assert(intputJson->top >= len);
    void* ret = intputJson->stack + intputJson->top - len;
    return ret;
}


//作用：将字符串按字节解析，将其保存在缓冲区中
int lept_parse_string(LeptJsonResult* result,  intput_json* inputJson){
    unsigned u;//unsigned int u
    unsigned u_low;
    size_t head = inputJson->top, len;
    const char* p;
    EXPECT_EQ(inputJson, '\"');
    p = inputJson->json;  //p用于保存josn的首地址指针
    for(;;){
        char ch = *p++;
        switch (ch) {
            case '\\':
                switch(*p++) {
                    case '\"':PUTC(inputJson, '\"');break;
                    case '\\':PUTC(inputJson, '\\');break;
                    case '/':PUTC(inputJson, '/');break;
                    case 'r':PUTC(inputJson, '\r');break;
                    case 'n':PUTC(inputJson, '\n');break;
                    case 't':PUTC(inputJson, '\t');break;
                    case 'b':PUTC(inputJson, '\b');break;
                    case 'f':PUTC(inputJson, '\f');break;
                    case 'u':
                        if (!(p = lept_parse_hex4(p, &u)))  //
                            TESTED_ERROR_STRING(inputJson, LEPT_PARSE_TYPE_INVALID_UNICODE);
                        if (u >= 0xd800 && u <= 0xdbff) {
                            if (*p++ != '\\')
                                TESTED_ERROR_STRING(inputJson, LEPT_PARSE_TYPE_INVALID_UNICODE);
                            if (*p++ != 'u')
                                TESTED_ERROR_STRING(inputJson, LEPT_PARSE_TYPE_INVALID_UNICODE);
                            if (!(p = lept_parse_hex4(p, &u_low)))
                                TESTED_ERROR_STRING(inputJson, LEPT_PARSE_TYPE_INVALID_UNICODE);
                            if (u_low > 0xdfff || u_low < 0xdc00)
                                TESTED_ERROR_STRING(inputJson, LEPT_PARSE_TYPE_INVALID_UNICODE);
                            u = 0x10000 + (u - 0xd800) * 0x400 + (u_low - 0xdc00);
                        }
                        lept_parse_utf8(inputJson, u);
                        break;
                    default:
                        TESTED_ERROR_STRING(inputJson, LEPT_PARSE_TYPE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\"':
                len = inputJson->top - head;
                lept_set_string(result, lept_context_pop(inputJson, len), len);
                inputJson->json = p;
                return LEPT_PARSE_OK;
            case '\0':  //因为json string格式为：\"******\",若是监测到\0说明后一个\"丢失了（未考虑字符串中间出现\0情况）
                inputJson->top = head;
                return LEPT_PARSE_MISS_QUOATION_MARK;
            default:
                if((unsigned char)ch < 0x20){//对照ascii码表,另外，char是否带符号是实现定义的，若编译器定义char带符号的话，（unsigned char）ch >= 0x80的字符都会变成负数，从而出错。在处理unicode时候需要考虑
                    TESTED_ERROR_STRING(inputJson, LEPT_PARSE_TYPE_ILLEGAL_STRING);
                }
                PUTC(inputJson, ch);
        }
    }
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
        case '"':
            return lept_parse_string(result, intputJson);
        case '[':
            return lept_parse_arrary(result,intputJson);
        default:
            return lept_parse_number(result, intputJson);
    }
}

//解析数组
int lept_parse_arrary(LeptJsonResult* result, intput_json *json){
        int size = 0;
        EXPECT_EQ(json,'[');
        lept_parse_whitespcae(json);//检查“[”后的空格
        if(*json->json == ']'){
            json->json++;
            result->leptjson_type = LEPT_ARRAY;
            result->a.size = 0;
            result->a.elemts = NULL;
            return LEPT_PARSE_OK;
        }
        for(;;){
            int ret = 0;
            LeptJsonResult leptJsonResult;
            leptJsonResult.leptjson_type = LEPT_NULL;
            if((ret = lept_parse_value(&leptJsonResult, json)) != LEPT_PARSE_OK)
            return ret;
            memcpy(lept_context_push(json, sizeof(LeptJsonResult)), &leptJsonResult, sizeof(leptJsonResult));
            size++;
            lept_parse_whitespcae(json); //检查数组元素后的空格
            //数组的一个元素解析完毕
            if(*json->json == ','){
                    json->json++;
                    lept_parse_whitespcae(json);//检查“]”前的空格
            }
            //数组解析完毕，将入栈的内容取出保存
            else if(*json->json == ']'){
                    json->json++;
                    result->leptjson_type = LEPT_ARRAY;
                    result->a.size = size;
                    size *= sizeof(LeptJsonResult);
                    memcpy(result->a.elemts = (LeptJsonResult*)malloc(size), lept_context_pop(json, size), size);
                return LEPT_PARSE_OK;
            } else{
                return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            }
        }

}
//根据utf-8码表解码
void lept_parse_utf8(intput_json *json, unsigned int u) {
    if(u >= 0 && u <= 0x007f){
        PUTC(json, u);
    }
    if(u >= 0x80 && u <=0x7ff){
        PUTC(json, 0xc0 | ((u >> 6) & 0xff));
        PUTC(json, 0x80 | (u       & 0x3f));
    }
    if(u >= 0x0800 && u <= 0xffff){
        PUTC(json, 0xe0 | ((u >> 12) & 0xff));
        PUTC(json, 0x80 | ((u >> 6) & 0x3f));
        PUTC(json, 0x80 | (u       & 0x3f));
    }
    if(u >= 0x10000 && u <= 0x10ffff){
        PUTC(json, 0xf0 | ((u >> 18) & 0xff));
        PUTC(json, 0x80 | ((u >> 12) & 0x3f));
        PUTC(json, 0x80 | ((u >> 6)  & 0x3f));
        PUTC(json, 0x80 | ((u        & 0x3f)));
    }
}


//string to ul
void* lept_parse_hex4(const char *p, unsigned int *pInt) {
   char ch;
   *pInt = 0;
   for(int i = 0; i <4; i++){
       ch = *p++;
       *pInt <<=4;
       if(ch >= '0' && ch <='9') *pInt |= ch - '0';
       else if(ch >= 'a' && ch <= 'f') *pInt |= ch - 'a' + 10;
       else if(ch >= 'A' && ch <= 'F') *pInt |= ch - 'A' + 10;
       else return NULL;
   }
    return p;
}


int LeptJson_Parse(LeptJsonResult *result, const char *InputJson){
    intput_json v;
    int ret;
    assert(result != NULL);
    v.json = InputJson;
    v.top = v.size = 0;
    v.stack = malloc(LEPT_PARSE_STACK_INIT_SIZE);
    result->leptjson_type = LEPT_INVALID;
    lept_parse_whitespcae(&v);
    if((ret = lept_parse_value(result, &v)) == LEPT_PARSE_OK){
        lept_parse_whitespcae(&v);
        if(*v.json != '\0'){
            result->leptjson_type = LEPT_INVALID;
            ret = LETP_PARSE_TYPE_NOT_SINGULAR;
        }
    }
    if(v.stack != NULL)
        free(v.stack);
    return ret;
}

int GetParseResult(const LeptJsonResult *result){
    assert(result != NULL);
    printf("parse leptjosn result:\n");
    printf("leptjosn type:%d\n", result->leptjson_type);
    if(result->leptjson_type == LEPT_NUMBER)
        printf("number:%.17g.\n", result->number);
    if(result->leptjson_type == LEPT_STRING){
        //printf("leptjosn type:%d, string: %s, size：%d", result->leptjson_type, result->s.string, (int)result->s.len);
    }
    return result->leptjson_type;
}

void lept_free(LeptJsonResult* leptJsonResult){
    assert(leptJsonResult != NULL);
    if(leptJsonResult->leptjson_type == LEPT_STRING){
        free(leptJsonResult->s.string);
    }
    leptJsonResult->leptjson_type = LEPT_NULL;
}

//将临时堆栈（缓冲区）中的字符保存到结果保存区（根据字符串长度len动态申请内存）
void lept_set_string(LeptJsonResult* leptJsonResult, const char* json, size_t len){
    //assert(leptJsonResult != NULL && json != NULL && len >= 0);
    lept_free(leptJsonResult);
    leptJsonResult->s.string = (char *)malloc(len+1);
    memcpy(leptJsonResult->s.string, json, len);
    leptJsonResult->s.len = len;
    leptJsonResult->s.string[len] = "\0";   //补上字符串结尾符
    leptJsonResult->leptjson_type = LEPT_STRING;
}

char* lept_get_string(LeptJsonResult* leptJsonResult){
    assert(leptJsonResult != NULL && leptJsonResult->leptjson_type == LEPT_STRING && leptJsonResult->s.string != NULL);
    return leptJsonResult->s.string;
}
size_t lept_get_strlen(LeptJsonResult* leptJsonResult){
    assert(leptJsonResult != NULL && leptJsonResult->leptjson_type == LEPT_STRING && leptJsonResult->s.string != NULL);
    return  leptJsonResult->s.len;
}


void lept_set_number(LeptJsonResult *leptJsonResult, double number) {
    assert(leptJsonResult != NULL);
    leptJsonResult->leptjson_type = LEPT_NUMBER;
    leptJsonResult->number = number;
}


double lept_get_number(LeptJsonResult *leptJsonResult) {
    assert(leptJsonResult != NULL && leptJsonResult->leptjson_type == LEPT_NUMBER);
    return leptJsonResult->number;
}

void lept_set_boolen(LeptJsonResult *leptJsonResult, int boolen) {
    assert(leptJsonResult != NULL);
    leptJsonResult->leptjson_type = boolen ? LEPT_TRUE:LEPT_FALSE;
}

LeptJsonType lept_get_boolen(LeptJsonResult* leptJsonResult){
    assert(leptJsonResult != NULL);
    return leptJsonResult->leptjson_type;
}

size_t lept_get_arrary_size(LeptJsonResult* leptJsonResult){
    assert(leptJsonResult != NULL);
    return  leptJsonResult->a.size;
}

LeptJsonResult* lept_get_arrary_by_index(LeptJsonResult* leptJsonResult, size_t index){
    assert(leptJsonResult != NULL);
    return (&leptJsonResult->a.elemts[index]);
}