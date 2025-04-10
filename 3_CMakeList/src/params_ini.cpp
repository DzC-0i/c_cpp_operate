#include <iostream>
#include <filesystem>
#include <vector>
/*
 * 读取的ini文件不一定要以.ini结尾，可以无后缀名方便使用.
 */
#include "inicpp.hpp"
#include "nlohmann/json.hpp"
// #include "params_ini.hpp"

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

int main()
{
    // 获取应用程序路径(fs::path(__FILE__)哪个.cpp文件启动的就是哪个的位置)
    fs::path APP_PATH = fs::absolute(fs::path(__FILE__).parent_path());

    // 获取项目路径（上一级目录）
    fs::path PRJ_PATH = APP_PATH.parent_path();

    // 添加下级目录位置
    fs::path properties = PRJ_PATH / "controller.properties";
    fs::path CONFIG_PATH = fs::absolute(PRJ_PATH / "config");
    fs::path PARAMS_PATH = fs::absolute(CONFIG_PATH / "params");

    // 读取目录中的文件名
    std::vector<std::string> filenames;
    for (const auto &entry : fs::directory_iterator(CONFIG_PATH))
    {
        filenames.push_back(entry.path().filename().string());
    }

    // 输出路径信息
    cout << "APP_PATH: " << APP_PATH << endl;
    cout << "PRJ_PATH: " << PRJ_PATH << endl;
    cout << "properties: " << properties << endl;
    cout << "CONFIG_PATH: " << CONFIG_PATH << endl;
    // 输出文件名
    cout << "--------------" << endl
         << "目录中的文件名: " << endl;
    for (const auto &name : filenames)
    {
        cout << name << endl;
    }
    std::cout << "--------------" << std::endl
              << "配置文件参数：" << std::endl;

    cout << "主配置文件：" << PARAMS_PATH / PARAMS_PATH.filename() << endl;
    // Load and parse the INI file.
    inicpp::IniManager config(PARAMS_PATH / PARAMS_PATH.filename());
    int count = config["PARAMS"]["count"];
    std::string param1 = config["PARAMS"]["param1"];
    std::string offset = config["BASEOFFSET"]["offset"];

    std::cout << "count: " << count << std::endl
              << "param1: " << param1 << std::endl
              << "offset: " << offset << std::endl;

    // 二级文件读取
    inicpp::IniManager config_params1(PARAMS_PATH / param1);
    int id = config_params1["DETAIL"]["id"];
    std::cout << "id: " << id << std::endl;

    // json解析在字符串
    json offsetJson = json::parse(offset);
    // 读取JSON中的各个字段
    double x = offsetJson["x"];
    double y = offsetJson["y"];
    double z = offsetJson["z"];
    double roll = offsetJson["roll"];
    double pitch = offsetJson["pitch"];
    double yaw = offsetJson["yaw"];

    // 输出结果
    std::cout << "x: " << x << std::endl
              << "y: " << y << std::endl
              << "z: " << z << std::endl
              << "roll: " << roll << std::endl
              << "pitch: " << pitch << std::endl
              << "yaw: " << yaw << std::endl;

    return 0;
}
