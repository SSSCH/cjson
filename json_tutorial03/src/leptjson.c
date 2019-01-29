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
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif


typedef struct {
    const char *json;
    char* stack; //动态数组
    size_t size, top; //@size 当前堆栈容量， @top 栈顶的位置
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
    errno = 0;
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
        if(errno == ERANGE && (result->number == HUGE_VAL || result->number == -HUGE_VAL)){
            result->leptjson_type = LEPT_INVALID;
            return LEPT_PARSE_TYPE_NUMBER_TOO_BIG;
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
                    default:
                        inputJson->top = head;
                        return LEPT_PARSE_TYPE_INVALID_STRING_ESCAPE;
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
                    inputJson->top =  head;
                    return LEPT_PARSE_TYPE_ILLEGAL_STRING;
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
        default:
            return lept_parse_number(result, intputJson);
    }
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