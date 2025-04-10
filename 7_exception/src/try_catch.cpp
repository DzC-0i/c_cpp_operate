#include <iostream>

using namespace std;

void test_try_catch()
{
    int m, n;
    try
    {
        cout << "input the m, n(1->int, 2->double, 3->msg, 4->all)" << endl;
        cin >> m >> n;
        if (n == 1)
            throw -1; // 抛出整型异常
        else if (n == 2)
            throw -1.2; // 拋出 double 型异常
        else if (n == 3)
            throw "The exception msg"; // 拋出 const char * 型异常
        else if (n < 0 || n > 9)
        {
            int c[2];
            c[0] = m;
            c[1] = n;
            throw c;
        }
        else
            cout << "m + n = " << m + n << endl;
    }
    // catch (const std::exception &e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
    catch (int e) // 捕获整型异常
    {
        std::cerr << e << '\n';
    }
    catch (double e) // 捕获 double 异常
    {
        std::cerr << e << '\n';
    }
    catch (const char *err_msg) // 捕获 const char * 异常
    {
        std::cerr << err_msg << '\n';
    }
    catch (...) // 匹配任何类型的异常
    {
        std::cerr << "unknow exception" << '\n';
    }
}

double division(int a, int b)
{
    if (b == 0)
        throw "division by zero condition!";
    return (double)a / b;
}

void func()
{
    try
    {
        int a, b;
        cout << "Input a and b: ";
        cin >> a >> b;
        cout << division(a, b) << "\n";
    }
    catch (...) // 这里捕获异常后并不处理异常，交给外面处理
    {
        throw;
    }
}

int main()
{
    try
    {
        // test_try_catch();
        func();
    }
    catch (const char *errmsg)
    {
        cout << errmsg << "\n";
    }
    catch (...)
    {
        cerr << "unknow exception" << '\n';
    }

    return 0;
}
