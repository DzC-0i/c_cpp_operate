#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

std::string getIPAddress()
{
    char hostname[256];
    std::string ipAddress = "Not Found";

    // 获取主机名
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        perror("gethostname");
        return ipAddress;
    }

    // 通过主机名获取主机信息
    struct hostent *host = gethostbyname(hostname);
    if (host == nullptr)
    {
        herror("gethostbyname");
        return ipAddress;
    }

    // 遍历地址列表，找到第一个非回环的IPv4地址
    for (int i = 0; host->h_addr_list[i] != nullptr; i++)
    {
        struct in_addr addr;
        memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));

        // 将地址转换为字符串
        char *ip = inet_ntoa(addr);
        if (ip != nullptr && std::string(ip) != "127.0.0.1")
        {
            ipAddress = ip;
            break;
        }
    }

    return ipAddress;
}

float getCPUUsage()
{
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    std::string cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long idle_time = idle + iowait;

    static long prev_total = 0, prev_idle = 0;
    long total_diff = total - prev_total;
    long idle_diff = idle_time - prev_idle;

    prev_total = total;
    prev_idle = idle_time;

    return 100.0f * (total_diff - idle_diff) / total_diff;
}

float getMemoryUsage()
{
    std::ifstream file("/proc/meminfo");
    std::string line;
    long total_memory = 0, free_memory = 0, available_memory = 0;
    while (std::getline(file, line))
    {
        if (line.find("MemTotal:") != std::string::npos)
        {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> total_memory;
        }
        else if (line.find("MemFree:") != std::string::npos)
        {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> free_memory;
        }
        else if (line.find("MemAvailable:") != std::string::npos)
        {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> available_memory;
        }
    }
    return 100.0f * (total_memory - available_memory) / total_memory;
}

float getDiskUsage()
{
    std::string command = "df / | awk 'NR==2 {print $5}'";
    std::string result;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
        return -1.0f;

    char buffer[128];
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    return std::stof(result.substr(0, result.size() - 1));
}

int main()
{
    std::string ipAddress = getIPAddress();
    float cpuUsage = getCPUUsage();
    float memoryUsage = getMemoryUsage();
    float diskUsage = getDiskUsage();

    std::cout << "IP Address: " << ipAddress << std::endl;
    std::cout << "CPU Usage: " << cpuUsage << "%" << std::endl;
    std::cout << "Memory Usage: " << memoryUsage << "%" << std::endl;
    std::cout << "Disk Usage: " << diskUsage << "%" << std::endl;

    return 0;
}
