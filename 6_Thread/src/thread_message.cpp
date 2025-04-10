#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

std::queue<int> messages;   // 消息队列
std::mutex mtx;             // 互斥锁
std::condition_variable cv; // 条件变量

bool stop = false;

void sender()
{
    for (size_t i = 0; i < 10; i++)
    {
        std::unique_lock<std::mutex> lock(mtx);
        messages.push(i); // 向队列中添加消息
        std::cout << "send" << std::endl;
        cv.notify_one(); // 通知等待的接收者
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 加点延时明显
    }
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true; // 设置停止标志
    }
    cv.notify_all(); // 通知所有等待的接收者
}

void receiver()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx); // 允许手动解锁
        cv.wait(lock, []
                { return !messages.empty() || stop; }); // 释放锁，等待条件变量通知，判断条件是否符合
        if (stop && messages.empty())
        {
            break; // 如果收到停止信号且队列为空，则退出循环
        }
        int msg = messages.front();                    // 获取消息
        messages.pop();                                // 从队列中移除消息
        lock.unlock();                                 // 解锁以允许其他线程访问队列
        std::cout << "Received: " << msg << std::endl; // 处理消息
    }
}

int main()
{
    std::thread sender_thread(sender);
    std::thread receiver_thread(receiver);

    sender_thread.join();
    receiver_thread.join();

    return 0;
}
