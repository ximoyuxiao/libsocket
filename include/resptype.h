#ifndef MY_RESP_H__
#define MY_RESP_H__
#include <nlohmann/json.hpp>
#include <macrogen.h>
class JsonType{
public:
    virtual void ToJson(nlohmann::json& nlohmann_json_j) = 0;
    virtual void FromJson(const nlohmann::json&nlohmann_json_j) = 0;
};
#endif