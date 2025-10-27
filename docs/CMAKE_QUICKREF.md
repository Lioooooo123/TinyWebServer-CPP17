# CMake 快速参考

## 一键构建

```bash
# Release 模式（推荐用于生产）
./cmake_build.sh

# Debug 模式（用于开发调试）
./cmake_build.sh -d

# 清理后构建
./cmake_build.sh -c

# 构建并安装
./cmake_build.sh -i

# 指定并行任务数
./cmake_build.sh -j 8
```

## 手动 CMake 命令

```bash
# 配置
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . -j$(nproc)

# 清理
rm -rf build/

# 安装
sudo cmake --install .
```

## 输出位置

- **可执行文件**: `build/bin/server`
- **配置文件**: `config/server.conf`
- **资源文件**: `root/`

## 运行服务器

```bash
# 从构建目录运行
cd build/bin
./server

# 使用配置文件
./server -f ../../config/server.conf

# 使用命令行参数
./server -p 9006 -t 8 -s 8
```

## 构建模式对比

| 模式 | 优化 | 调试信息 | 用途 |
|------|------|----------|------|
| Release | `-O2` | 无 | 生产环境 |
| Debug | 无 | `-g` | 开发调试 |

## IDE 配置

### VS Code
安装 `CMake Tools` 扩展后：
- `Ctrl+Shift+P` → "CMake: Configure"
- `F7` 编译
- `Shift+F5` 调试

### CLion
直接打开项目文件夹，自动识别 CMakeLists.txt

## 常见问题

### 找不到 MySQL
```bash
# 安装开发库
sudo apt-get install libmysqlclient-dev

# 或指定路径
cmake .. -DMYSQL_INCLUDE_DIR=/path/to/mysql/include
```

### CMake 版本太低
```bash
# 检查版本
cmake --version

# 升级 CMake
sudo apt-get install cmake
# 或从官网下载最新版本
```

### 更改编译器
```bash
# 使用 Clang
cmake .. -DCMAKE_CXX_COMPILER=clang++

# 使用特定版本的 GCC
cmake .. -DCMAKE_CXX_COMPILER=g++-11
```

## 更多信息

完整文档：[CMAKE_MIGRATION.md](CMAKE_MIGRATION.md)
