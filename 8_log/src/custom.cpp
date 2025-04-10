#include <iostream>

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

int main(int argc, char const *argv[])
{
    ColorLogger::log(ColorLogger::Level::INFO, "This is INFO...");
    ColorLogger::log(ColorLogger::Level::WARNING, "This is WARNING...");
    ColorLogger::log(ColorLogger::Level::ERROR, "This is ERROR");

    return 0;
}
