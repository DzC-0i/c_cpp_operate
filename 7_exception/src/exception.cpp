#include <iostream>
#include <string>
#include "Error.hpp"

// 自定义 log工具类
class ColorLogger
{
public:
    enum class Level
    {
        INFO,
        WARNING,
        ERROR
    };

    static void log(Level level, const std::string &message)
    {
        switch (level)
        {
        case Level::INFO:
            std::cout << "\033[32m[INFO]  \t" << message << "\033[0m" << std::endl;
            break;
        case Level::WARNING:
            std::cerr << "\033[33m[WARNING]\t" << message << "\033[0m" << std::endl;
            break;
        case Level::ERROR:
            std::cerr << "\033[31m[ERROR]  \t" << message << "\033[0m" << std::endl;
            break;
        }
    }
};

void connectToDevice()
{
    // 模拟连接失败
    throw ConnError(ErrorCode::CONTROLLER_ERROR_DRIVER_CONN, "Connection timeout");
}

void processData(int param)
{
    if (param < 0)
    {
        throw ParamError(ErrorCode::CONTROLLER_ERROR_PARAMS, "Invalid parameter value");
    }
    // ... 其他操作
}

int main()
{
    try
    {
        ColorLogger::log(ColorLogger::Level::INFO, "Exception start!");
        ColorLogger::log(ColorLogger::Level::WARNING, "Exception processData in...");
        // connectToDevice();
        processData(-1);
    }
    // 单独使用每个的错误来处理
    // catch (const ConnError &e)
    // {
    //     std::cerr << e.what() << std::endl;
    //     // 处理连接错误
    // }
    // catch (const ParamError &e)
    // {
    //     std::cerr << e.what() << std::endl;
    //     // 处理参数错误
    // }
    // catch (const BaseException &e)
    // {
    //     std::cerr << e.what() << std::endl;
    //     // 处理其他派生异常
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "Standard Exception: " << e.what() << std::endl;
    //     // 处理标准异常
    // }

    // 最外层可以直接用 BaseException 基类来处理和显示错误。
    catch (const BaseException &e)
    {
        ColorLogger::log(ColorLogger::Level::ERROR, e.what());
    }
    catch (const std::exception &e)
    {
        std::cerr << "Standard Exception: " << e.what() << std::endl;
        // 处理标准异常
    }
    catch (...)
    {
        std::cerr << "Unkown Exception" << std::endl;
    }
    return 0;
}
