//
// Created by SCH on 2019/1/19.
//

#ifndef JSON_TUTORIAL01_LEPTJSON_H
#define JSON_TUTORIAL01_LEPTJSON_H
//待解析的json类型
typedef enum{
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_INVALID
}LeptJsonType;
//Josn 结果的容器
typedef struct {
    double number;
    LeptJsonType leptjson_type
}LeptJsonResult;
//解析完成后返回值类型
typedef enum{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_NO_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LETP_PARSE_TYPE_NOT_SINGULAR,
    LEPT_PARSE_TYPE_ILLEGAL_NUMBER
}RetType;
//解析器
int LeptJson_Parse(LeptJsonResult *result, const char *InputJson);

//获取解析结果
int GetParseResult(const LeptJsonResult *result);
#endif //JSON_TUTORIAL01_LEPTJSON_H
