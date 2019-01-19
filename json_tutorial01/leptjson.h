//
// Created by SCH on 2019/1/17.
//

#ifndef JSON_TUTORIAL01_LEPTJSON_H
#define JSON_TUTORIAL01_LEPTJSON_H
//json type，只解析前三种,默认为LEPT_NULL,解析成功就更改为相应的格式
typedef enum  {LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_OBJECT, LEPT_ARRAY, LEPT_STRING, LEPT_NUMBER}lept_type;

typedef struct {
    lept_type type;
}type_value;

//return type
enum  {
    LEPT_PARSE_ok = 0,
    LEPT_PARSE_EXPECT_VALUE, //内容为空
    LEPT_PARSE_INVALID_VALUE, //无法解析的格式，不属于前三种
    LEPT_PARSE_ROOT_NOOT_SINGULAR,// 有多余字符
};

//解析器
int leptJson_parse(type_value *result, const char* json);

//获取解析结果
lept_type get_leptJsonType(const type_value * value);

#endif //JSON_TUTORIAL01_LEPTJSON_H
