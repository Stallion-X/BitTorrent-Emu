# BitTorrent-Emu
An emulation of BitTorrent by Win32 Socket.

初版实现，之后有空会重构和增加重传机制

初次使用需要修改宏定义的路径

每个Peer代码逻辑均一样，实现对等P2P连接

每个Peer可以自由选择进行做种或者下载

下载时会先读取.torrent文件，拿到Tracker的网络地址，再逐个文件块请求Tracker服务器，从Tracker获取对应文件块所在服务器的网络地址，并从对应地址下载对应文件块，下载全部完成后进行合并
