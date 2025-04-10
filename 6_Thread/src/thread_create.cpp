#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic> //原子操作

using namespace std;
std::mutex mtx;
int n = 0;

std::atomic<int> atom{0};

void foo(size_t z)
{
    for (int i = 1; i <= 10; i++)
    {
        atom++;
        // cout << "1";
    }
    for (size_t i = 0; i < z; i++)
    {
        {
            std::lock_guard<std::mutex> lock(mtx); // 使用锁保护区域，防止输出错位,出作用域后自动释放
            n++;
            cout << "*  \t函数指针-互斥锁：全局变量 n 的值: " << n << endl;
            cout << "线程使用函数指针作为可调用参数，第 " << i + 1 << "次调用" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// 需要传递引用参数，需要使用 std::ref
// 也可以使用 std::cref ，以const的形式传递参数
void increment(size_t x, size_t &ret)
{
    for (int i = 1; i <= 10; i++)
    {
        atom++;
        // cout << "2";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ret = x * x;

    std::lock_guard<std::mutex> lock(mtx); // 使用锁保护区域，防止输出错位，出作用域后自动释放
    n++;
    cout << "*  \t传递引用参数-互斥锁：全局变量 n 的值: " << n << endl;
    cout << "- 引用地：引用的参数值改动后: " << ret << endl;
}

// 通过类中的 operator() 方法定义函数对象来创建线程
class ThreadObj
{
public:
    void operator()(size_t x) const
    {
        for (int i = 1; i <= 10; i++)
        {
            atom++;
            // cout << "3";
        }
        for (size_t i = 0; i < x; i++)
        {
            {
                std::lock_guard<std::mutex> lock(mtx); // 使用锁保护区域，防止输出错位，出作用域后自动释放
                n++;
                cout << "*  \t函数对象-互斥锁：全局变量 n 的值: " << n << endl;
                cout << "线程使用函数对象作为可调用参数，第 " << i + 1 << "次调用" << endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

void countnumber(int id, unsigned int n)
{
    for (unsigned int i = 1; i <= n; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 延时函数切换有开销，延时短的时候开销整体特别明显
    // std::lock_guard<std::mutex> lock(mtx);
    // cout << "Thread " << id + 1 << " finished!" << endl;
}

int main()
{
    size_t num = 3, ret;
    cout << "- 原地：引用的参数值改动前: " << ret << endl;
    cout << "*  \t原地：全局变量 n 的值改动前: " << n << endl;

    thread th1(foo, 3);
    thread th2(increment, num, std::ref(ret));
    thread th3(ThreadObj(), 3);
    // 使用 Lambda 表达式创建线程
    thread th4([](size_t x)
               {
                for (int i = 1; i <= 10; i++)
                {
                    atom++;
                    // cout << "4";
                }
                for (size_t i = 0; i < x; i++){
                    {
                        std::lock_guard<std::mutex> lock(mtx); // 使用锁保护区域，防止输出错位，出作用域后自动释放
                        n++;
                        cout << "*  \tlambda 表达式-互斥锁：全局变量 n 的值: " << n << endl;
                        cout << "线程使用 lambda 表达式作为可调用参数，第 " << i+1 << "次调用" << endl;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                } }, 3);
    th1.join();
    th2.join();
    th3.join();
    th4.join();

    cout << "Thread1-4 joined!\nOther thread start!" << endl;

    const unsigned int core = std::thread::hardware_concurrency(); // 获取 CPU 核心数
    thread thr[core];
    // 获取当前系统时间点,高精度时间点
    auto now = std::chrono::high_resolution_clock::now();

    for (unsigned int i = 0; i < core; i++)
        thr[i] = thread(countnumber, i, 10);
    for (unsigned int i = 0; i < core; i++)
        thr[i].join();

    // 获取当前系统时间点
    auto now2 = std::chrono::high_resolution_clock::now();

    cout << "任务数量：" << core << endl
         << "运行时长：" << std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count() << " ms" << endl;

    cout << "- 原地：引用的参数值改动后: " << ret << endl;
    cout << "*  \t原地：全局变量 n 的值改动后: " << n << endl;
    cout << "原子操作: " << atom << endl;
    return 0;
}
