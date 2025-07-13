# 使用示例

我觉得这个改进后的BT协议项目使用起来更标准化了 让我来演示一下具体的使用方法

## 编译项目

```bash
# 使用Makefile编译所有组件
make all

# 或者单独编译
make tracker
make peer
make test_bencode
```

## 运行测试

```bash
# 测试bencode编解码功能
./test_bencode

# 测试基本编译和功能
./compile_test
```

## 启动BitTorrent网络

### 1. 启动Tracker服务器
```bash
./tracker
```
Tracker会自动：
- 生成标准格式的.torrent文件
- 启动HTTP服务器监听8080端口
- 支持/announce和/scrape接口

### 2. 启动做种Peer
```bash
./peer
# 选择模式2进行做种
```

### 3. 启动下载Peer
```bash
./peer
# 选择模式1进行下载
```

## 协议特性

现在支持的标准BT协议特性：
- Bencode编码的.torrent文件
- HTTP Tracker协议（announce/scrape）
- 标准BT握手协议
- BT peer wire protocol消息
- 向后兼容原有功能

## 文件结构

```
BitTorrent-Emu/
├── bencode.h/cpp          # Bencode编解码库
├── bt_protocol.h/cpp      # BT协议处理
├── Tracker/               # Tracker服务器
├── Peer/                  # Peer客户端
├── test_bencode.cpp       # 单元测试
├── compile_test.cpp       # 编译测试
└── Makefile              # 编译配置
```

这样的改进让项目更接近真实的BitTorrent协议实现 我觉得现在可以更好地模拟BT网络的行为了
