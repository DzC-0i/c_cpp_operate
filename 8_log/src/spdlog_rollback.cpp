#define SPDLOG_ACTIVE_LEVEL SPDLOG_LOGGER_TRACE
#include <iostream>
#include <spdlog/spdlog.h>
// 输出log日志文件
#include <spdlog/sinks/basic_file_sink.h>
// 文件路径操作时导入 C++17支持
#include <filesystem>

namespace fs = std::filesystem;

// 按照文件大小回滚日志
#include "spdlog/sinks/rotating_file_sink.h"
void rotating_example()
{
    // // Create a file rotating logger with 5mb size max and 3 rotated files.
    // auto max_size = 1024 * 1024 * 5;
    // auto max_files = 3;
    // auto rotating_logger =
    //     spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.log", max_size, max_files);

    auto rotating_logger =
        spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.log", 1024, 3);

    // 设置默认log的输出
    spdlog::set_default_logger(rotating_logger);

    // 输出有 [日志标识符] [日期] [日志级别] [文件位置: 行数] [数据]
    rotating_logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l](%@): %v"); // 非通过宏输出的日志%@输出为空
    rotating_logger->set_level(spdlog::level::info);

    rotating_logger->info("Hello, {}!", "World");
    SPDLOG_LOGGER_INFO(rotating_logger, "Welcome to info rotating_logger");
    SPDLOG_LOGGER_WARN(rotating_logger, "Welcome to warn rotating_logger");
    SPDLOG_LOGGER_ERROR(rotating_logger, "Welcome to error rotating_logger");

    // 输出到默认日志中
    spdlog::error("Some error message with arg: {}", 1);
    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::info("Support for floats {:03.2f}", 1.23456);
}

// 按照日期回滚日志
#include "spdlog/sinks/daily_file_sink.h"
void daily_example()
{
    // Create a daily logger - a new file is created every day on 2:30am.
    // 参数含义: [日志标识符] [文件位置] [刷新新文件: 时] [刷新新文件: 分] [是否截断] [最大文件数量]
    auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.log", 2, 30, false, 3);

    // 设置日志格式. 参数含义: [日志标识符] [日期] [日志级别] [线程号] [文件名 函数名:行号] [数据]
    daily_logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#]  %v");

    // 日志等级设置
    daily_logger->set_level(spdlog::level::info);
    spdlog::flush_every(std::chrono::seconds(5)); // 定期刷新日志缓冲区

    daily_logger->info("Hello, {}!", "World");
    SPDLOG_LOGGER_INFO(daily_logger, "Welcome to info daily_logger");
    SPDLOG_LOGGER_WARN(daily_logger, "Welcome to warn daily_logger");
    SPDLOG_LOGGER_ERROR(daily_logger, "Welcome to error daily_logger");

    // 输出到默认日志中
    spdlog::error("[daily_logger] Some error message with arg: {}", 1);
    spdlog::warn("[daily_logger] Easy padding in numbers like {:08d}", 12);
    spdlog::info("[daily_logger] Support for floats {:03.2f}", 1.23456);
}

#include <spdlog/sinks/hourly_file_sink.h>
void hour_example()
{
    // 参数含义: [日志标识符] [文件位置] [是否截断] [最大文件数量]
    auto hourly_logger = spdlog::hourly_logger_mt("hourly_logger", "logs/hourly.log", false, 3);
    // 设置日志格式. 参数含义: [日志标识符] [日期] [日志级别] [线程号] [文件名 函数名:行号] [数据]
    hourly_logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#]  %v");

    // 日志等级设置
    hourly_logger->set_level(spdlog::level::info);

    hourly_logger->info("Hello, {}!", "World");
    SPDLOG_LOGGER_INFO(hourly_logger, "Welcome to info daily_logger");
    SPDLOG_LOGGER_WARN(hourly_logger, "Welcome to warn daily_logger");
    SPDLOG_LOGGER_ERROR(hourly_logger, "Welcome to error daily_logger");

    // 输出到默认日志中
    spdlog::error("[hourly_logger] Some error message with arg: {}", 1);
    spdlog::warn("[hourly_logger] Easy padding in numbers like {:08d}", 12);
    spdlog::info("[hourly_logger] Support for floats {:03.2f}", 1.23456);
}

int main()
{
    rotating_example();
    daily_example();
    hour_example();

    // 其他地方写入log
    std::shared_ptr<spdlog::logger> mylogger = spdlog::get("hourly_logger");
    SPDLOG_LOGGER_INFO(mylogger, "The hourly_logger in main");

    return 0;
}
