/* 如果需要打印文件名、函数名和行号，必须在文件开头定义以下宏。如果要在多个文件中打印,
 * 也必须在文件开头定义这个宏。否则只会打印默认级别，set_level 设置的日志级别不会生效。
 * #define SPDLOG_ACTIVE_LEVEL SPDLOG_LOGGER_TRACE
 * 并且要使用 SPDLOG_LOGGER* 系列函数打印
 */
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LOGGER_TRACE
#include <iostream>
#include <spdlog/spdlog.h>
// 输出log日志文件
#include <spdlog/sinks/basic_file_sink.h>
// 文件路径操作时导入 C++17支持
#include <filesystem>

namespace fs = std::filesystem;

void myFuncTest()
{
    // 其他位置可以通过日志标识符来定位日志的输出
    std::shared_ptr<spdlog::logger> mylogger = spdlog::get("spdlog_file");

    mylogger->trace("Welcome to myFuncTest!");
    mylogger->debug("Welcome to myFuncTest!");
    mylogger->info("Welcome to myFuncTest!");
    mylogger->warn("Welcome to myFuncTest!");
    mylogger->error("Welcome to myFuncTest!");
}

int main(int argc, char const *argv[])
{
    try
    {
        // 初始化日志输出. 参数含义: [日志标识符] [文件位置]
        // // 直接填写是在终端运行相同目录下生成这个文件
        // std::shared_ptr<spdlog::logger> my_logger = spdlog::basic_logger_mt("spdlog", "basic.txt");
        // 获取当前工作目录(终端打开位置在哪里，哪里就是WORK_PATH)
        fs::path WORK_PATH = fs::current_path();
        // 输出路径信息
        fs::path LOG_PATH = fs::absolute(WORK_PATH / "logs");
        // 创建新目录(存在就不创建)
        fs::create_directory(LOG_PATH);
        // 输出路径信息
        std::cout << "LOG_PATH: " << LOG_PATH << std::endl;

        std::shared_ptr<spdlog::logger> my_logger = spdlog::basic_logger_mt("spdlog_file", LOG_PATH / "spdlog_file.log");

        // 设置日志格式. 参数含义: [日志标识符] [日期] [日志级别] [线程号] [文件名 函数名:行号] [数据]
        my_logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#]  %v");

        // 日志等级设置
        my_logger->set_level(spdlog::level::info);
        spdlog::flush_every(std::chrono::seconds(5)); // 定期刷新日志缓冲区

        // 没有行号时这个正常使用, 也可以输出，但是没有行号的部分
        // my_logger->info("Welcome to info spdlog!");

        // 需要行号的时候要使用： SPDLOG_LOGGER* 系列函数打印
        SPDLOG_LOGGER_TRACE(my_logger, "Welcome to trace spdlog");
        SPDLOG_LOGGER_DEBUG(my_logger, "Welcome to debug spdlog");
        SPDLOG_LOGGER_INFO(my_logger, "Welcome to info spdlog");
        SPDLOG_LOGGER_WARN(my_logger, "Welcome to warn spdlog");
        SPDLOG_LOGGER_ERROR(my_logger, "Welcome to error spdlog");
        // SPDLOG_LOGGER_CRITICAL(my_logger, "Welcome to critical spdlog");

        myFuncTest();

        // 刷新
        my_logger->flush();
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }

    return 0;
}
