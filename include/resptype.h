#ifndef MY_RESP_H__
#define MY_RESP_H__
#include <nlohmann/json.hpp>
#define NLOHMANN_CLASS_JSON_TO(v1) nlohmann_json_j[#v1] = this->v1;
#define NLOHMANN_CLASS_JSON_FROM(v1) nlohmann_json_j.at(#v1).get_to(this->v1);

#define REGISTER_DEFINE_JSON_SERIALIZATION(Type, ...)  \
    void Type::to_json(nlohmann::json& nlohmann_json_j) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_CLASS_JSON_TO, __VA_ARGS__)) } \
    void Type::from_json(const nlohmann::json& nlohmann_json_j) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_CLASS_JSON_FROM, __VA_ARGS__)) }

class JsonType{
public:
    virtual void to_json(nlohmann::json& nlohmann_json_j) = 0;
    virtual void from_json(const nlohmann::json&nlohmann_json_j) = 0;
};

#endif