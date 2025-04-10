#include <iostream>
#include "spdlog/spdlog.h"

// 以彩色方式输出到标准输出设备上
#include "spdlog/sinks/stdout_color_sinks.h"
void stdout_example()
{
    // create color multi threaded logger
    auto console = spdlog::stdout_color_mt("console");
    auto err_logger = spdlog::stderr_color_mt("stderr");
    // // 这行会产生警告，但是可以使用
    // spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");
    err_logger->error("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");
}

int main(int argc, char const *argv[])
{
    // 普通打印
    // 设置日志格式，在日志文件测试当中有写
    // trace、debug 默认级别低看不见
    // 设置打印级别
    spdlog::set_level(spdlog::level::debug);

    // 下面几个级别递增
    spdlog::trace("This is spdlog trace..");
    spdlog::debug("This is spdlog debug..");
    // 格式化打印
    // 打印字符串
    spdlog::info("Hello world {}", ", this is spdlog!");
    // 打印数字
    spdlog::warn("Spdlog print number: {}", -12345);
    // 打印数字的占位符
    spdlog::error("Spdlog ErrorCode (format char): {:08d}", 12);
    // 打印不同进制格式的数据
    spdlog::critical("Spdlog support for int: {0:d}, hex: {0:x}, oct: {0:o}, bin: {0:b}", 42);

    // 打印浮点数据
    spdlog::info("Spdlog print float: {:03.2f}", 1.23456);
    spdlog::info("Spdlog print float: {:09.3f}", 1.23456);
    // 打印多个参数
    spdlog::info("Spdlog print multiple parameters: {0} . {1} . {2} . {3}...", "one", "two", 123, -123);
    spdlog::info("Spdlog print multiple parameters: {0:d}, hex: {0:x}, oct: {1:o}, bin: {1:b}", 42, 43);

    std::cout << "------------------------\n";
    stdout_example();

    return 0;
}
