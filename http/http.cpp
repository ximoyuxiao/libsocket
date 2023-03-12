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
std::unordered_map<int,std::string> statusStr{
    {200,"OK"},
    {400,"Bad Request"},
    {403 , "Forbidden"},
    {404 , "Not Found"},
    {500 , "Internal Error"},
};
/*
    { error_400_form , "Your request has bad syntax or is inherently impossible to satisfy.\n"},
    { error_403_form , "You do not have permission to get file from this server.\n"},
    { error_404_form , "The requested file was not found on this server.\n"},
    { error_500_form , "There was an unusual problem serving the requested file.\n"},
*/
};