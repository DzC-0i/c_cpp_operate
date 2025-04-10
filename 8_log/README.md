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
