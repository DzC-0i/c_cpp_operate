# **【异常安全】**
- 构造函数完成对象的构造和初始化，最好不要在构造函数中抛出异常，否则可能导致对象不完整或没有完全初始化
- 析构函数主要完成资源的清理，最好不要在析构函数内抛出异常，否则可能导致资源泄漏(内存泄漏、句柄未关闭等)
- C++中异常经常会导致资源泄漏的问题，比如在 new 和 delete 中抛出了异常，导致内存泄漏，在lock 和 unlock 之间抛出了异常导致死锁，C++经常使用RAII来解决以上问题

# **【异常规范】**
- 异常规格说明的目的是为了让函数使用者知道该函数可能抛出的异常有哪些。 可以在函数的后面接throw(类型)，列出这个函数可能抛掷的所有异常类型。
- 函数的后面接 throw()，表示函数不抛异常。
- 若无异常接口声明，则此函数可以抛掷任何类型的异常。

```c++
// 这里表示这个函数会抛出A/B/C/D中的某种类型的异常
void fun() throw(A，B，C，D);
 // 这里表示这个函数只会抛出bad_alloc的异常
void* operator new (std::size_t size) throw (std::bad_alloc);
 // 这里表示这个函数不会抛出异常
void* operator delete (std::size_t size, void* ptr) throw();
 // C++11 中新增的noexcept，表示不会抛异常
thread() noexcept;
thread (thread&& x) noexcept;
```

# `mutable` 关键字注意事项

1. 不要滥用 mutable：

- 仅用于真正不影响对象逻辑状态的场景（如缓存、日志）。

- 滥用会破坏 const 的正确性，导致代码难以维护。

2. 线程安全性：

- 多个线程调用 const 成员函数时，对 mutable 成员的修改需额外同步（如用 std::mutex）。

3. 与 const_cast 的区别：

- mutable 是类型系统的一部分，安全且可控。

- const_cast 是强制去除 const，可能导致未定义行为（UB）。


# cmake 使用

步骤

1. 在项目根目录运行以下命令：

```bash
cmake -B build
```

或者

```bash
cd build/
cmake ..
```
这会在 build 文件夹中生成构建系统。

2. 编译项目：

```bash
cmake --build build
```

或者

```bash
cd build/
cmake --build .
```

或者

```bash
cd build/
make
```

```bash
# -j6：多核并行编译，可以提高编译速度,不要超过核心数目
make -j6
```

3. 运行生成的可执行文件：

```bash
./build/bin/params_ini
./build/bin/params
```

4. 清理文件

```bash
make clean
```

**优点**

- 完全避免污染项目根目录。

- 符合 CMake 的推荐用法。
