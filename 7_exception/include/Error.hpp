#pragma once
#include <exception>
#include "ErrorCodes.hpp"

class BaseException : public std::exception
{
protected:
    std::string message_title; // 错误类型标题（如 "ConnError"）
    std::string message_code;  // 错误码（如 "EC-00-0001"）
    std::string message_text;  // 错误描述文本

    // 允许在 const 成员函数中修改该成员变量
    mutable std::string full_msg; // 缓存完整的错误信息

public:
    BaseException(const std::string &title, const std::string &code, const std::string &text)
        : message_title(title), message_code(code), message_text(text) {}

    // 返回完整的错误信息
    virtual const char *what() const noexcept override
    {
        if (full_msg.empty())
        {
            full_msg = message_title + " " + message_code + ": " + message_text;
        }
        return full_msg.c_str();
    }

    // Getters
    std::string getTitle() const { return message_title; }
    std::string getCode() const { return message_code; }
    std::string getText() const { return message_text; }
};

// 连接错误
class ConnError : public BaseException
{
public:
    ConnError(const std::string &code, const std::string &text)
        : BaseException("ConnError", code, text) {}
};

// 通信错误
class CommError : public BaseException
{
public:
    CommError(const std::string &code, const std::string &text)
        : BaseException("CommError", code, text) {}
};

// 动态库错误
class DllError : public BaseException
{
public:
    DllError(const std::string &code, const std::string &text)
        : BaseException("DllError", code, text) {}
};

// 断言错误
class AssertError : public BaseException
{
public:
    AssertError(const std::string &code, const std::string &text)
        : BaseException("AssertError", code, text) {}
};

// 执行错误
class ExecuteError : public BaseException
{
public:
    ExecuteError(const std::string &code, const std::string &text)
        : BaseException("ExecuteError", code, text) {}
};

// 配置文件错误
class ConfigError : public BaseException
{
public:
    ConfigError(const std::string &code, const std::string &text)
        : BaseException("ConfigError", code, text) {}
};

// 参数错误
class ParamError : public BaseException
{
public:
    ParamError(const std::string &code, const std::string &text)
        : BaseException("ParamError", code, text) {}
};

// 单位错误、计算异常
class UnitError : public BaseException
{
public:
    UnitError(const std::string &code, const std::string &text)
        : BaseException("UnitError", code, text) {}
};
