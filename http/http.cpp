#include<http.h>
using namespace my;
namespace my{
#if  __cplusplus < 201703L
const HttpMethod_t HttpMethod::CONNECT = "CONNECT";
const HttpMethod_t HttpMethod::DELETE  = "DELETE";
const HttpMethod_t HttpMethod::GET = "GET";
const HttpMethod_t HttpMethod::HEAD = "HEAD";
const HttpMethod_t HttpMethod::OPTIONS = "OPTIONS";
const HttpMethod_t HttpMethod::PATCH = "PATCH";
const HttpMethod_t HttpMethod::POST = "POST";
const HttpMethod_t HttpMethod::PUT = "PUT";
const HttpMethod_t HttpMethod::TRACE = "TRACE";
#endif

std::unordered_map<std::string,HttpMethod_t> methods{
    {"POST",HttpMethod::POST},
    {"CONNECT",HttpMethod::CONNECT},
    {"DELETE",HttpMethod::DELETE},
    {"GET",HttpMethod::GET},
    {"HEAD",HttpMethod::HEAD},
    {"OPTIONS",HttpMethod::OPTIONS},
    { "PATCH",HttpMethod::PATCH},
    {"POST",HttpMethod::POST},
    {"PUT",HttpMethod::PUT},
    {"TRACE",HttpMethod::TRACE},
};
};