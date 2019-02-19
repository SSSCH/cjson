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
#define PUTS(p, s, len)      do{memcpy(lept_context_push(p, len), s, len);} while(0)
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
    void* ret = intputJson->stack + (intputJson->top -= len);
    return ret;
}

int _lept_parse_string(char** result, intput_json* inputJson, size_t* len){
    unsigned u;//unsigned int u
    unsigned u_low;
    size_t head = inputJson->top;
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
                *len = inputJson->top - head;
                *result = lept_context_pop(inputJson, *len);
                //memcpy(*result = (char*)malloc(*len), lept_context_pop(inputJson, *len), *len);
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

//作用：将字符串按字节解析，将其保存在缓冲区中
int lept_parse_string(LeptJsonResult* result, intput_json* inputJson){
    int ret = 0;
    char* string = "";
    size_t len = 0;
    if((ret = _lept_parse_string(&string, inputJson, &len)) != LEPT_PARSE_OK){
        return ret;
    }
    lept_set_string(result, string, len);
    return ret;
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
        case '{':
            return lept_parse_object(result, intputJson);
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
int lept_parse_object(LeptJsonResult* result, intput_json* json){
            size_t size = 0; //member count
            int ret = 0;
            char* string = "";
            LeptJsonMember leptJsonMember; //防止内存泄漏的重要原则：在哪一层/哪一地方创建的变量/申请的内存，就在那边的结尾记得free（）；
            EXPECT_EQ(json, '{');
            for(;;){
                lept_parse_whitespcae(json);
                if(*json->json == '}'){
                    json->json++;
                    result->leptjson_type = LEPT_OBJECT;
                    result->o.m = NULL;
                    result->o.size = 0;
                    return LEPT_PARSE_OK;
                    }
                //解析键
                if(*json->json != '"'){
                    ret = LEPT_PAESE_MISS_KEY;
                    break;//end for(;;)
                }
                leptJsonMember.k = NULL; //將其初始化为空，这样在解析成功后下一次的循环进来不会受到影响
                if((ret=_lept_parse_string(&string, json, &leptJsonMember.klen)) != LEPT_PARSE_OK){
                    ret = LEPT_PARSE_TYPE_ILLEGAL_STRING;
                    break;
                }
                //将memb的key值保存到临时变量
                memcpy(leptJsonMember.k = (char*)malloc(leptJsonMember.klen), string, leptJsonMember.klen);
                leptJsonMember.k[leptJsonMember.klen] = '\0'; //添加字符串結束標志
                lept_parse_whitespcae(json);
                //检查冒号：
                if(*json->json != ':'){
                    ret = LEPT_PARSE_MISS_KEY_VALUE;
                    break;
                }
                json->json++; //jump ：
                lept_parse_whitespcae(json);
                //到这一步说明对象成员解析成功，此时键值保存在临时变量（kvalue）中
                if((ret = lept_parse_value(&leptJsonMember.kvalue, json)) != LEPT_PARSE_OK){
                    break;
                }
                //將保存在临时变量的leptjsonmember即时压栈
                memcpy(lept_context_push(json, sizeof(leptJsonMember)), &leptJsonMember, sizeof(leptJsonMember));
                size++; //leptjsonmember count+1
                lept_parse_whitespcae(json);
                //遇到逗号，
                if(*json->json == ','){
                    json->json++;
                    lept_parse_whitespcae(json);
                }
                //遇到右花括号,对象解析结束
                else if(*json->json == '}'){
                        json->json++;
                        size_t s = sizeof(leptJsonMember)*size; //s为对象占用总字节数
                        result->leptjson_type = LEPT_OBJECT;
                        result->o.size = size;
                        memcpy((result->o.m = (LeptJsonMember*)malloc(s)), lept_context_pop(json, s), s);
                    return LEPT_PARSE_OK;
                }
                else{
                    ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
                    break;
                }
            }
  //出现任何错误将走到这里
    if (leptJsonMember.k != NULL)
        free(leptJsonMember.k);//回收临时变量的堆栈
   for (int i = 0; i < size; ++i) { //回收自定义的堆栈size个leptjosnmember
       LeptJsonMember* Member = (LeptJsonMember*)lept_context_pop(json, sizeof(leptJsonMember));
       free(&Member->k);
       lept_free(&Member->kvalue);
   }
    result->leptjson_type = LEPT_NULL;
    return ret;
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
    v.top = 0;
    v.size = LEPT_PARSE_STACK_INIT_SIZE;
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
    switch (leptJsonResult->leptjson_type) {
        case LEPT_STRING:
            free(leptJsonResult->s.string);
            break;
        case LEPT_ARRAY:
            for (size_t i = 0; i <leptJsonResult->a.size; ++i) {
                lept_free(&leptJsonResult->a.elemts[i]);
            }
            free(leptJsonResult->a.elemts);
            break;
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
    assert(leptJsonResult != NULL);
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
char* lept_get_member_key(LeptJsonResult* leptJsonResult, size_t index){
    assert((leptJsonResult != NULL) && (index <= leptJsonResult->o.size));
    return  leptJsonResult->o.m[index].k;
}
size_t lept_get_member_keylen(LeptJsonResult* leptJsonResult, size_t index){
    assert((leptJsonResult != NULL) && (index <= leptJsonResult->o.size));
    return leptJsonResult->o.m[index].klen;
}
LeptJsonResult* lept_get_member(LeptJsonResult* leptJsonResult, size_t index){
    assert((leptJsonResult != NULL) && (index <= leptJsonResult->o.size));
    return &leptJsonResult->o.m[index].kvalue;
}
void _lept_stringfy_string(const char* string, size_t  len ,intput_json* tmpstack){
    for (int i = 0; i < len; ++i) {
        unsigned char s = (string[i]);
        switch(string[i]){
            case '\"': PUTS(tmpstack, "\\\"", 2);break;
            case '\\': PUTS(tmpstack, "\\\\", 2);break;
            case '\t': PUTS(tmpstack, "\\t", 2);break;
            case '\r': PUTS(tmpstack, "\\r", 2);break;
            case '\n': PUTS(tmpstack, "\\n", 2);break;
            case '\f': PUTS(tmpstack, "\\f", 2);break;
            default:
                if(s < 0x20){
                    sprintf(lept_context_push(tmpstack, 6), "\\u%04x", s);break;
                } else{
                    PUTC(tmpstack, s);
                    break;
                }
        }

    }
}
int _lept_stringfy(LeptJsonResult* jsonResult, intput_json* tmpstack){
    switch (jsonResult->leptjson_type) {
        case LEPT_FALSE :
            PUTS(tmpstack,"false", 5);break;
        case LEPT_NULL:
            PUTS(tmpstack,"null", 4);break;
        case LEPT_TRUE :
            PUTS(tmpstack,"true", 4);break;
        case LEPT_NUMBER :
            tmpstack->top -= 32 - sprintf(lept_context_push(tmpstack, 32), "%.7g", jsonResult->number);break;
            // tmpstack.top = tmpstack.top -32 + sprintf(lept_context_push(&tmpstack, 32), "%.17g", jsonResult->number);
        case LEPT_OBJECT:
            PUTC(tmpstack, '{');
            for (int j = 0; j < jsonResult->o.size; ++j) {
                if (j != 0) {PUTC(tmpstack, ',');}
                PUTC(tmpstack, '"');
                _lept_stringfy_string(jsonResult->o.m[j].k, jsonResult->o.m[j].klen, tmpstack);
                PUTC(tmpstack,'\"');
                PUTC(tmpstack, ':');
                if(!_lept_stringfy(&jsonResult->o.m[j].kvalue, tmpstack))
                    return -1;
            }
            PUTC(tmpstack, '}');
            break;
        case LEPT_STRING:
            PUTC(tmpstack, '\"');
            _lept_stringfy_string(jsonResult->s.string,jsonResult->s.len, tmpstack);
            //PUTS(&tmpstack, jsonResult->s.string, jsonResult->s.len);
            PUTC(tmpstack, '\"');
            break;
        case LEPT_ARRAY:
            PUTC(tmpstack, '[');
            for (int i = 0; i <jsonResult->a.size; ++i) {
                if (i != 0) {PUTC(tmpstack, ',');}
                _lept_stringfy(&jsonResult->a.elemts[i], tmpstack);
            }
            PUTC(tmpstack, ']');
            break;
        default :
            assert(0 && "LEPT_INVALID");
            return -1;
    }
}
int lept_stringfy(LeptJsonResult* jsonResult, char** ResultString, size_t* len){
    intput_json tmpstack;
    tmpstack.top = 0;
    tmpstack.size = LEPT_PARSE_STACK_INIT_SIZE;
    tmpstack.stack = malloc(LEPT_PARSE_STACK_INIT_SIZE);
    if(!_lept_stringfy(jsonResult, &tmpstack))
    return -1;
    *len = tmpstack.top;
    *ResultString = (char*)malloc(tmpstack.size);
    memcpy(*ResultString, tmpstack.stack, tmpstack.top);
    (*ResultString)[tmpstack.top] = '\0';
    if(tmpstack.stack != NULL)
        free(tmpstack.stack);
    return LEPt_PASE_STRINGFY_OK;
}