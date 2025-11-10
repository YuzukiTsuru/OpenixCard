# OpenixCard 使用指南

## 程序位置

编译后的可执行文件位于：`build/dist/OpenixCard`

## 基本使用方法

### 1. 查看帮助信息
```bash
./build/dist/OpenixCard --help
# 或
./build/dist/OpenixCard -h
```

### 2. 查看版本信息
```bash
./build/dist/OpenixCard --version
# 或
./build/dist/OpenixCard -v
```

## 主要功能

### 功能 1: 解包 Allwinner 镜像文件 (-u)

**用途**：将 Allwinner 格式的固件镜像文件解包，提取其中的分区和文件。

**使用方法**：
```bash
./build/dist/OpenixCard -u <镜像文件路径>
```

**示例**：
```bash
# 解包名为 firmware.img 的镜像文件
./build/dist/OpenixCard -u firmware.img

# 解包后会生成一个同名的目录，包含所有解包的文件
```

**输出**：
- 创建一个目录（通常与镜像文件名相同）
- 目录中包含所有分区和文件内容

---

### 功能 2: 解包并生成分区表配置 (-uc)

**用途**：解包镜像文件，同时生成分区表配置文件（.cfg），用于后续重新打包。

**使用方法**：
```bash
./build/dist/OpenixCard -uc <镜像文件路径>
```

**示例**：
```bash
./build/dist/OpenixCard -uc firmware.img
```

**输出**：
- 解包后的文件目录
- 分区表配置文件（.cfg），记录了分区的布局信息

**使用场景**：
- 需要修改固件后重新打包
- 需要了解镜像的分区结构
- 备份分区表配置

---

### 功能 3: 转储镜像 (-d)

**用途**：将 Allwinner 专有格式的镜像转换为标准的镜像格式。

**使用方法**：
```bash
./build/dist/OpenixCard -d <镜像文件路径>
```

**示例**：
```bash
./build/dist/OpenixCard -d firmware.img
```

**输出**：
- 转换后的标准格式镜像文件
- 通常可以直接用于刷机或挂载

**使用场景**：
- 将 Allwinner 格式转换为通用格式
- 提取原始镜像数据
- 用于其他工具处理

---

### 功能 4: 获取镜像大小 (-s)

**用途**：获取 Allwinner 镜像文件的准确大小信息。

**使用方法**：
```bash
./build/dist/OpenixCard -s <镜像文件路径>
```

**示例**：
```bash
./build/dist/OpenixCard -s firmware.img
```

**输出**：
- 显示镜像的准确大小（字节数或带单位的大小）

**使用场景**：
- 验证镜像文件完整性
- 了解镜像大小以便准备存储设备
- 调试镜像文件问题

---

### 功能 5: 打包镜像 (-p)

**用途**：从解包后的目录重新打包成 Allwinner 镜像文件。

**使用方法**：
```bash
./build/dist/OpenixCard -p <目录路径>
```

**示例**：
```bash
# 假设已经解包了 firmware.img，生成了 firmware 目录
./build/dist/OpenixCard -p firmware
```

**前置条件**：
- 目录中必须包含 `.cfg` 配置文件
- 目录结构必须完整（通常由 `-uc` 命令生成）

**输出**：
- 重新打包的镜像文件

**使用场景**：
- 修改固件后重新打包
- 创建自定义固件镜像
- 测试镜像打包功能

---

## 完整工作流程示例

### 场景 1: 分析和修改固件

```bash
# 1. 解包镜像并生成配置文件
./build/dist/OpenixCard -uc firmware.img

# 2. 这会生成 firmware 目录和配置文件
# 3. 修改 firmware 目录中的文件
# 4. 重新打包
./build/dist/OpenixCard -p firmware

# 5. 生成新的镜像文件
```

### 场景 2: 转换镜像格式

```bash
# 1. 转储为标准格式
./build/dist/OpenixCard -d firmware.img

# 2. 可以使用其他工具处理转换后的镜像
```

### 场景 3: 检查镜像信息

```bash
# 1. 查看镜像大小
./build/dist/OpenixCard -s firmware.img

# 2. 解包查看内容
./build/dist/OpenixCard -u firmware.img
```

## 注意事项

1. **文件路径**：确保镜像文件路径正确，可以使用相对路径或绝对路径
2. **权限**：确保对目标目录有写入权限
3. **磁盘空间**：解包操作需要足够的磁盘空间
4. **配置文件**：打包时需要 `.cfg` 文件，建议使用 `-uc` 选项生成
5. **备份**：在修改固件前，建议先备份原始镜像文件

## 常见问题

**Q: 如何知道镜像文件是否有效？**
A: 使用 `-s` 选项检查镜像大小，或尝试 `-u` 解包看是否成功。

**Q: 打包时提示缺少配置文件？**
A: 使用 `-uc` 选项解包，会自动生成 `.cfg` 配置文件。

**Q: 可以处理哪些类型的镜像？**
A: 主要用于 Allwinner 芯片的 Linux 固件镜像，不支持 Android 固件。

## 快速参考

```bash
# 查看帮助
./build/dist/OpenixCard -h

# 查看版本
./build/dist/OpenixCard -v

# 解包镜像
./build/dist/OpenixCard -u <img>

# 解包并生成配置
./build/dist/OpenixCard -uc <img>

# 转储镜像
./build/dist/OpenixCard -d <img>

# 获取大小
./build/dist/OpenixCard -s <img>

# 打包镜像
./build/dist/OpenixCard -p <dir>
```

