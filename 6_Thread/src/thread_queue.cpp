#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <cmath>

std::mutex mtx;
std::condition_variable cv;
std::vector<std::queue<int>> dataQueues; // 每个线程有自己的数据队列
std::vector<int> results;                // 存储处理后的结果
bool finished = false;

void worker(int id, int index)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]
                { return !dataQueues[index].empty() || finished; });

        if (finished && dataQueues[index].empty())
        {
            break;
        }

        if (!dataQueues[index].empty())
        {
            int data = dataQueues[index].front();
            dataQueues[index].pop();
            lock.unlock();

            // 模拟数据处理（乘方）
            int result = std::pow(data, 2);

            std::cout << "Thread " << id << ": " << data << " -> " << result << std::endl;
            // 将结果存储到对应的位置
            std::lock_guard<std::mutex> outputLock(mtx);
            results[index] = result;
        }
    }
}

int main()
{
    std::vector<std::thread> threads;
    int numThreads = 4; // 假设有4个线程

    // 初始化数据队列和结果向量
    std::vector<int> data = {12, 15, 18, 20};
    std::vector<int> data2 = {3, 6, 9, 12};
    dataQueues.resize(numThreads);
    results.resize(data.size());

    // 创建子线程，每个线程负责一个固定的数据索引
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(worker, i + 1, i);
    }

    // 下发数据到对应的线程队列
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::lock_guard<std::mutex> lock(mtx);
        dataQueues[i].push(data[i]);
        dataQueues[i].push(data2[i]);
        // cv.notify_all(); // 通知所有线程检查自己的队列
    }

    // 通知子线程数据下发完成
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
        cv.notify_all(); // 通知所有线程检查自己的队列
    }

    // 等待子线程完成
    for (auto &t : threads)
    {
        t.join();
    }

    // 输出处理后的结果
    std::cout << "Processed results: ";
    for (int result : results)
    {
        std::cout << result << " ";
    }
    std::cout << std::endl;

    return 0;
}
