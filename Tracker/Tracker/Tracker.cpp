#include <windows.h>
#include <iostream>
#include <winsock.h>
#include <string>
#include <sstream>
#include <map>
#pragma comment(lib,"ws2_32.lib")
#define NO_FLAGS_SET 0
#define PUBLIC_IP "127.0.0.1"
#define PORT (u_short) 8080
#define MAXBUFLEN sizeof(chunk_pack)
#define FILELIST_MAXSIZE 100
#define INPUT_NAME "test.jpg"
#define OUTPUT_NAME "output.jpg"
#define CHUNK_COUNT 10
#define CHUNK_MAXSIZE 1000000
#define PATH "C:\\Users\\only\\Desktop\\server\\"
#define TORRENT_PATH "C:\\Users\\only\\Desktop\\"

using namespace std;

void init_data();
void init();
void recv_request();
void process_request(char *);
void split_ip(char *);
void torrent_generator(int index);

typedef struct
{
	int index;
	unsigned char file_chunk[CHUNK_MAXSIZE];
}chunk_pack;
typedef struct
{
	char name[1000];
	long long file_size;
	int chunk_count;
	map<int, std::string>chunk_addr;
}file_meta;
file_meta file_list[FILELIST_MAXSIZE];
WSADATA Data;
SOCKADDR_IN serverSockAddr;
SOCKADDR_IN clientSockAddr;
SOCKET serverSocket;
SOCKET clientSocket;
int addrLen = sizeof(SOCKADDR_IN);
int status;
int numrcv;
char buffer[MAXBUFLEN];
long long file_size;
char ip[100];
int port;
int main(void)
{
	init_data();
	torrent_generator(0);
	init();
	recv_request();
	
}
void init()
{
	cout << "Starting Tracker..." << endl;
	/* initialize the Windows Socket DLL */
	status = WSAStartup(MAKEWORD(1, 1), &Data);
	/*初始化 Winsock DLL
	 if (status != 0)
	 cerr << "ERROR: WSAStartup unsuccessful" << endl;
	 /* zero the sockaddr_in structure */
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	/* specify the port portion of the address */
	serverSockAddr.sin_port = htons(PORT);
	/* specify the address family as Internet */
	serverSockAddr.sin_family = AF_INET;
	/* specify that the address does not matter */
   /*INADDR_ANY 的具体含义是，绑定到 0.0.0.0。此时，对所有的地址都将是有效的
   serverSockAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	53
	/* create a socket */
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
		cerr << "ERROR: socket unsuccessful" << endl;
	/* associate the socket with the address */
	status = bind(serverSocket, (LPSOCKADDR)&serverSockAddr,
		sizeof(serverSockAddr));
	if (status == SOCKET_ERROR)
		cerr << "ERROR: bind unsuccessful" << endl;
	/* allow the socket to take connections */
	status = listen(serverSocket, 1);
	if (status == SOCKET_ERROR)
		cerr << "ERROR: listen unsuccessful" << endl;
	/* accept the connection request when one is received */
	clientSocket = accept(serverSocket,
		(LPSOCKADDR)&clientSockAddr,
		&addrLen);
	cout << "Got the connection..." << endl;
}
//本来应该是Tracker服务器的数据库 此处为硬编码
void init_data()
{
	memcpy(file_list[0].name, "test.jpg", sizeof("test.jpg"));
	file_list[0].file_size = 19944;
	file_list[0].chunk_count = CHUNK_COUNT;
	file_list[0].chunk_addr.emplace(0, "169.254.168.122:44965");
	file_list[0].chunk_addr.emplace(1, "169.254.168.122:44965");
	file_list[0].chunk_addr.emplace(2, "169.254.168.122:44965");
	file_list[0].chunk_addr.emplace(3, "169.254.249.177:44965");
	file_list[0].chunk_addr.emplace(4, "169.254.249.177:44965");
	file_list[0].chunk_addr.emplace(5, "169.254.249.177:44965");
	file_list[0].chunk_addr.emplace(6, "169.254.150.154:44965");
	file_list[0].chunk_addr.emplace(7, "169.254.150.154:44965");
	file_list[0].chunk_addr.emplace(8, "169.254.150.154:44965");
	file_list[0].chunk_addr.emplace(9, "169.254.150.154:44965");
	//cout << file_list[0].chunk_addr.find(0)->second << endl;
	memcpy(file_list[1].name, "test.mp3", sizeof("test.mp3"));
	file_list[1].file_size = 19944;
	file_list[1].chunk_count = CHUNK_COUNT;
	file_list[1].chunk_addr.emplace(0, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(1, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(2, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(3, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(4, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(5, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(6, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(7, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(8, "150.158.145.94:44965");
	file_list[1].chunk_addr.emplace(9, "150.158.145.94:44965");
	for (int i = 10; i <= 100; i++)
	{
		file_list[1].chunk_addr.emplace(i, "150.158.145.94:44965");
	}
}
void recv_request()
{
	cout << "Receiving Requests..." << endl;
	char buf1[MAXBUFLEN];
	while (1)
	{
		numrcv = recv(clientSocket, buffer, sizeof(buffer), NO_FLAGS_SET);
		if ((numrcv == 0) || (numrcv == SOCKET_ERROR))
		{
			cout << "Connection terminated." << endl;
			status = closesocket(clientSocket);
			if (status == SOCKET_ERROR)
				cerr << "ERROR: closesocket unsuccessful"
				<< endl;
			status = WSACleanup();
			if (status == SOCKET_ERROR)
				cerr << "ERROR: WSACleanup unsuccessful"
				<< endl;
			init();
			continue;
		}
		cout << string("Received file name: ") << buffer << endl;
		memcpy(buf1, buffer, size(buffer));
		memset(buffer, 0, MAXBUFLEN);
		Sleep(100);

		numrcv = recv(clientSocket, buffer, sizeof(buffer), NO_FLAGS_SET);
		if ((numrcv == 0) || (numrcv == SOCKET_ERROR))
		{
			cout << "Connection terminated." << endl;
			status = closesocket(clientSocket);
			if (status == SOCKET_ERROR)
				cerr << "ERROR: closesocket unsuccessful"
				<< endl;
			status = WSACleanup();
			if (status == SOCKET_ERROR)
				cerr << "ERROR: WSACleanup unsuccessful"
				<< endl;
			init();
			continue;
		}
		process_request(buf1);
	} /* while */
}
void process_request(char *buf)
{
	int index;
	map<int, std::string>mapper;
	string addr;
	for (int i = 0; i < FILELIST_MAXSIZE; i++)
	{
		if (strcmp(buf, file_list[i].name) == 0)
		{
			mapper = file_list[i].chunk_addr;
			stringstream strIn;
			strIn << buffer;
			strIn >> index;
			cout << string("Received file chunk index: ") << index << endl;
			memset(buffer, 0, MAXBUFLEN);
			addr = mapper.find(index)->second;
			break;
		}
	}
	send(clientSocket, addr.c_str(),strlen(addr.c_str()) + 1, NO_FLAGS_SET);
}
void split_ip(char *addr)
{
	sscanf(addr, "%[^:]:%d", ip, &port);
}
void torrent_generator(int index)
{
	FILE *torrent;
	string path = TORRENT_PATH + string("test") + ".torrent";
	if ((torrent = fopen(path.c_str(), "w+")) == NULL) {  //以文本写入/更新方式打开 创建新文件
		cerr << "Fail to open file!";
		exit(0);
	}
	string content = file_list[index].name + string("\n") + PUBLIC_IP + string("\n") + to_string(PORT);
	fputs(content.c_str(), torrent);
	fclose(torrent);
}