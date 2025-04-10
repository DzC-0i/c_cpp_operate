查看线程库, GNU C库（glibc）的POSIX线程库（pthread）的实现名称和版本

```bash
getconf GNU_LIBPTHREAD_VERSION
```

1. `getconf` 的作用
这是一个用于查询系统配置参数的命令，例如文件路径长度限制、内存页大小等。通过指定参数（如 GNU_LIBPTHREAD_VERSION），可以获取特定配置的值。

2. `GNU_LIBPTHREAD_VERSION` 的含义
该参数专门用于查询 GNU C库（glibc）的POSIX线程库（pthread）的实现名称和版本。

示例输出：

```bash
dingzc@dingzc-VM:~$ getconf GNU_LIBPTHREAD_VERSION
NPTL 2.39
```

cmake 使用

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
