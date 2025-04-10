#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <map>
// #include "params.hpp"

using namespace std;
namespace fs = std::filesystem;

// 去除字符串两端的空格和制表符
std::string trim(const std::string &str)
{
    const char *whitespace = " \t";
    size_t start = str.find_first_not_of(whitespace);
    size_t end = str.find_last_not_of(whitespace);
    if (start == std::string::npos || end == std::string::npos)
    {
        return "";
    }

    return str.substr(start, end - start + 1);
}

// 解析配置文件
std::map<std::string, std::string> parseConfigFile(const std::string &filePath)
{
    std::map<std::string, std::string> config;
    std::ifstream file(filePath);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return config;
    }

    while (std::getline(file, line))
    {
        // 去除行首尾的空格和制表符
        line = trim(line);

        // 忽略空行和注释行
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        // 查找等号的位置
        size_t equalsPos = line.find('=');
        if (equalsPos == std::string::npos)
        {
            continue; // 如果没有等号，跳过该行
        }

        // 解析键和值
        std::string key = trim(line.substr(0, equalsPos));
        std::string value = trim(line.substr(equalsPos + 1));

        // 存储到 map 中
        config[key] = value;
    }

    file.close();
    return config;
}

// 添加新行到配置文件，保留对齐格式
void addConfigLine(const std::string &filePath, const std::string &key, const std::string &value)
{
    std::ifstream inFile(filePath);
    std::ofstream outFile(filePath + ".tmp");
    std::string line;
    bool keyExists = false;

    if (!inFile.is_open() || !outFile.is_open())
    {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return;
    }

    // 逐行读取原文件
    while (std::getline(inFile, line))
    {
        std::string trimmedLine = trim(line);

        // 如果是注释行或空行，直接写入
        if (trimmedLine.empty() || trimmedLine[0] == '#')
        {
            outFile << line << std::endl;
            continue;
        }

        // 查找等号的位置
        size_t equalsPos = trimmedLine.find('=');
        if (equalsPos == std::string::npos)
        {
            outFile << line << std::endl;
            continue;
        }

        // 解析键
        std::string currentKey = trim(trimmedLine.substr(0, equalsPos));

        // 如果键已存在，更新值
        if (currentKey == key)
        {
            outFile << key << "=" << value << std::endl; // 保持对齐
            keyExists = true;
        }
        else
        {
            outFile << line << std::endl;
        }
    }

    // 如果键不存在，追加到文件末尾
    if (!keyExists)
    {
        outFile << key << "=" << value << std::endl;
    }

    inFile.close();
    outFile.close();

    // 替换原文件
    std::remove(filePath.c_str());
    std::rename((filePath + ".tmp").c_str(), filePath.c_str());
}

int main()
{
    // 获取应用程序路径(fs::path(__FILE__)哪个.cpp文件启动的就是哪个的位置)
    fs::path APP_PATH = fs::absolute(fs::path(__FILE__).parent_path());

    // 获取项目路径（上一级目录）
    fs::path PRJ_PATH = APP_PATH.parent_path();

    // 添加下级目录位置
    fs::path properties = PRJ_PATH / "controller.properties";
    fs::path CONFIG_PATH = fs::absolute(PRJ_PATH / "config");

    // 读取目录中的文件名
    std::vector<std::string> filenames;
    for (const auto &entry : fs::directory_iterator(CONFIG_PATH))
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
    cout << "APP_PATH: " << APP_PATH << endl;
    cout << "PRJ_PATH: " << PRJ_PATH << endl;
    cout << "properties: " << properties << endl;
    cout << "CONFIG_PATH: " << CONFIG_PATH << endl;

    auto config = parseConfigFile(properties);
    // 输出解析结果
    for (const auto &pair : config)
    {
        std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }

    // 添加或更新键值对
    std::string newKey = "COMPO_add_PATH";
    std::string newValue = "~/learn";
    addConfigLine(properties, newKey, newValue);
    std::cout << "已添加或更新键值对: " << newKey << " = " << newValue << std::endl;

    return 0;
}
