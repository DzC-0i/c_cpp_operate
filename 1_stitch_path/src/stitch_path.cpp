#include <iostream>
#include <filesystem>
#include <vector>
// #include "stitch_path.hpp"

using namespace std;
namespace fs = std::filesystem;

int main()
{
    // 获取 HOME 路径
    fs::path HOME_PATH = getenv("HOME"); // Linux/macOS
#ifdef _WIN32
    HOME_PATH = getenv("USERPROFILE"); // Windows
#endif

    // 获取当前工作目录(终端打开位置在哪里，哪里就是WORK_PATH)
    fs::path WORK_PATH = fs::current_path();

    // 获取应用程序路径(fs::path(__FILE__)哪个.cpp文件启动的就是哪个的位置)
    fs::path APP_PATH = fs::absolute(fs::path(__FILE__).parent_path());

    // 获取项目路径（上一级目录）
    fs::path PRJ_PATH = APP_PATH.parent_path();

    // 添加下级目录位置
    fs::path INCLUDE_PATH = PRJ_PATH / "include";
    fs::path CONFIG_PATH = fs::absolute(PRJ_PATH / "config");

    // 创建新目录(存在就不从创建)
    fs::create_directory(CONFIG_PATH);

    // 读取目录中的文件名
    std::vector<std::string> filenames;
    for (const auto &entry : fs::directory_iterator(INCLUDE_PATH))
    {
        filenames.push_back(entry.path().filename().string());
    }
    // 输出文件名
    cout << "目录中的文件名: ";
    for (const auto &name : filenames)
    {
        cout << name << endl;
    }

    // 输出路径信息
    cout << "HOME_PATH: " << HOME_PATH << endl;
    cout << "WORK_PATH: " << WORK_PATH << endl;
    cout << "APP_PATH: " << APP_PATH << endl;
    cout << "PRJ_PATH: " << PRJ_PATH << endl;
    cout << "INCLUDE_PATH: " << INCLUDE_PATH << endl;
    cout << "CONFIG_PATH: " << CONFIG_PATH << endl;
    return 0;
}
