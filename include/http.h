#ifndef HTTP_H__
#define HTTP_H__
#include<unordered_map>
namespace my{
typedef int HttpStatus_t;
typedef int HttpMode_t;
typedef std::string HttpMethod_t;
enum HTTP_CHECK_STATE_t{
    CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT
};
enum HTTP_LINE_STATUS_t{
    LINE_OK = 0, LINE_BAD, LINE_OPEN 
};
/*
    服务器处理HTTP请求的可能结果，报文解析的结果
    NO_REQUEST          :   请求不完整，需要继续读取客户数据
    GET_REQUEST         :   表示获得了一个完成的客户请求
    BAD_REQUEST         :   表示客户请求语法错误
    CLOSED_CONNECTION   :   表示客户端已经关闭连接了
    INTERNAL_ERROR      :   服务器内部错误
*/
enum HTTP_RESULT_t{
    NO_REQUEST, GET_REQUEST, BAD_REQUEST, CLOSED_CONNECTION,INTERNAL_ERROR
};

class HttpStatus{
public:
//
    static const int StatusContinue = 100;
    static const int StatusSwitchingProtocol = 101;
    static const int StatusProcessing = 102;
    static const int StatusEarlyHints = 103; // 暂不可使用
    //
    static const int StatusOK = 200;
    static const int StatusCreated = 201;
    static const int StatusAccepted = 202;
    static const int StatusNonAuthoritativeInformation = 203;
    static const int StatusNoContent = 204;
    static const int StatusResetContent = 205;
    static const int StatusPartialContent = 206;
    static const int StatusMultiStatus = 207;
    static const int StatusAlreadyReported = 208;
    static const int StatusIMUsed =  226;
    //
    static const int StatusMultipleChoices = 300;
    static const int StatusMovedPermanently = 301;
    static const int StatusFound = 302;
    static const int StatusSeeOther = 303;
    static const int StatusNotModified = 304;
    static const int StatusTemporaryRedirect = 307;
    static const int StatusPermanentRedirect = 308;

    static const int StatusBadRequest = 400;
    static const int StatusUnauthorized = 401;
    static const int StatusPaymentRequired = 402;
    static const int StatusForbidden = 403;
    static const int StatusNotFound = 404;
    static const int StatusMethodNotAllowed = 405;
    static const int StatusNotAcceptable = 406;
    static const int StatusProxyAuthenticationRequired = 407;
    static const int StatusRequestTimeout = 408;
    static const int StatusConflict = 409;
    static const int StatusGone = 410;
    static const int StatusLengthRequired = 411;
    static const int StatusPreconditionFailed = 412;
    static const int StatusContentTooLarge = 413;
    static const int StatusURITooLong = 414;
    static const int StatusUnsupportedMediaType = 415;
    static const int StatusRangeNotSatisfiable = 416;
    static const int StatusExpectationFailed = 417;
    static const int StatusATeapot = 418;
    static const int StatusMisdirectedRequest = 421;
    static const int StatusUnprocessableContent = 422;
    static const int StatusLocked = 423;
    static const int StatusFailedDependency = 424;
    static const int StatusTooEarly = 425;
    static const int StatusUpgradeRequired = 426;
    static const int StatusPreconditionRequired = 428;
    static const int StatusTooManyRequests = 429;
    static const int StatusRequestHeaderFieldsTooLarge = 431;
    static const int StatusUnavailableForLegalReasons = 451;
    //
    static const int StatusInternalServerError = 500;
    static const int StatusNotImplemented = 501;
    static const int StatusBadGateway = 502;
    static const int StatusServiceUnavailable = 503;
    static const int StatusGatewayTimeout = 504;
    static const int StatusHTTPVersionNotSupported = 505;
    static const int StatusVariantAlsoNegotiates = 506;
    static const int StatusInsufficientStorage = 507;
    static const int StatusLoopDetected = 508;
    static const int StatusNotExtended = 510;
    static const int StatusNetworkAuthenticationRequired = 511;
};

class HttpMode{
public:
    static const int  M_Default = 0;
    static const int  M_Debug = 1;
    static const int  M_Release = 2;
};

class HttpMethod{
public:
#if  __cplusplus >= 201703L
    inline static const HttpMethod_t CONNECT = "CONNECT";
    inline static const HttpMethod_t DELETE  = "DELETE";
    inline static const HttpMethod_t GET = "GET";
    inline static const HttpMethod_t HEAD = "HEAD";
    inline static const HttpMethod_t OPTIONS = "OPTIONS";
    inline static const HttpMethod_t PATCH = "PATCH";
    inline static const HttpMethod_t POST = "POST";
    inline static const HttpMethod_t PUT = "PUT";
    inline static const HttpMethod_t TRACE = "TRACE";
#else
    static const HttpMethod_t CONNECT;
    static const HttpMethod_t DELETE  ;
    static const HttpMethod_t GET ;
    static const HttpMethod_t HEAD ;
    static const HttpMethod_t OPTIONS ;
    static const HttpMethod_t PATCH ;
    static const HttpMethod_t POST;
    static const HttpMethod_t PUT;
    static const HttpMethod_t TRACE;
#endif
};
extern std::unordered_map<std::string,HttpMethod_t> methods;
extern std::unordered_map<int,std::string> statusStr;
};
#endif