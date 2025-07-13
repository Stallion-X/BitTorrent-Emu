#include <windows.h>
#include <iostream>
#include <winsock.h>
#include <string>
#include <sstream>
#include "../../bencode.h"
#include "../../bt_protocol.h"
#pragma comment(lib,"ws2_32.lib")
#define NO_FLAGS_SET 0
#define PORT (u_short) 44965
#define PUBLIC_IP "127.0.0.1"
#define DEST_IP_ADDR "127.0.0.1" //Server address
#define PATH "C:\\Users\\only\\Desktop\\test\\"
#define SAVE_PATH "C:\\Users\\only\\Desktop\\save\\"
#define TORRENT_PATH "C:\\Users\\only\\Desktop\\"
#define INPUT_NAME "test.jpg"
#define OUTPUT_NAME "output.jpg"
#define CHUNK_COUNT 10
#define CHUNK_MAXSIZE 1000000
using namespace std;

int init_peer_connection();
int init_tracker_connection();
void init_server();
int connect();
void torrent_reader();
long long get_file_size(FILE *fp);
void file_slice();
void save_chunks();
void read_chunks();
void merge_chunks();
void send_header();
void send_chunks();
void send_single_chunk(int i);
void split_ip(char *addr);
void request_tracker(int index);
void recv_header();
void recv_chunks();
int recv_single_chunk();
void loop_mode_recv();
void request_single_chunk(int index);
void recv_chunk_request();
void send_bt_handshake();
void recv_bt_handshake();
void send_bt_message(unsigned char msg_id, const char* payload, int payload_len);
void recv_bt_message();
void send_bt_interested();
void send_bt_request(int piece_index, int block_offset, int block_length);

WSADATA Data;
SOCKADDR_IN destSockAddr;
SOCKET destSocket;
unsigned long destAddr;
SOCKADDR_IN serverSockAddr;
SOCKADDR_IN clientSockAddr;
SOCKET serverSocket;
SOCKET clientSocket;
int addrLen = sizeof(SOCKADDR_IN);
int status;
int numsnt;
int numrcv;
char tracker_ip[100];
USHORT tracker_port;
typedef struct
{
	int index;
	//unsigned char* file_chunk;//弃用 因为指针类型作为成员就不能直接强转用socket传过去
	unsigned char file_chunk[CHUNK_MAXSIZE];
}chunk_pack;
#define MAXBUFLEN sizeof(chunk_pack)
char buffer[MAXBUFLEN];
chunk_pack chunk_list[CHUNK_COUNT + 1];
chunk_pack new_chunk_list[CHUNK_COUNT + 1];
long long file_size;
char filename[100] = "test.jpg";
char peer_ip[100];
USHORT peer_port;

INT main(VOID)
{
	int mode;
	cout << "请选择Peer模式：1.分块下载模式 2.做种模式" << endl;
	cin >> mode;
	switch (mode)
	{
	case 1:
		torrent_reader();
		for (int i = 0; i < CHUNK_COUNT; i++)
		{
			request_tracker(i);
			request_single_chunk(i);
		}
		read_chunks();
		merge_chunks();
		break;
	case 2:
		FILE *fp;
		string path = PATH + string(INPUT_NAME);
		if ((fp = fopen(path.c_str(), "rb+")) == NULL) {  //以二进制读写方式打开
			cerr << "Fail to open file!";
			exit(0);
		}
		file_size = get_file_size(fp);
		file_slice();
		save_chunks();
		init_server();
		recv_chunk_request();
		break;
	}
	return 0;
}
int init_peer_connection()
{
	/* initialize the Windows Socket DLL */
	status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
		cerr << "ERROR: WSAStartup unsuccessful"
		<< endl;
	/* convert IP address into in_addr form */
	destAddr = inet_addr(peer_ip);
	/* copy destAddr into sockaddr_in structure */
	memcpy(&destSockAddr.sin_addr,
		&destAddr, sizeof(destAddr));
	/* specify the port portion of the address */
	destSockAddr.sin_port = htons(peer_port);
	/* specify the address family as Internet */
	destSockAddr.sin_family = AF_INET;
	/* create a socket */
	destSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (destSocket == INVALID_SOCKET)
	{
		cerr << "ERROR: socket unsuccessful" << endl;
		status = WSACleanup();
		if (status == SOCKET_ERROR)
			cerr << "ERROR: WSACleanup unsuccessful"
			<< endl;
		return(1);
	}
	cout << "Trying to connect to IP Address: "
		<< peer_ip <<":" << peer_port << endl;
}
int init_tracker_connection()
{
	/* initialize the Windows Socket DLL */
	status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
		cerr << "ERROR: WSAStartup unsuccessful"
		<< endl;
	/* convert IP address into in_addr form */
	destAddr = inet_addr(tracker_ip);
	/* copy destAddr into sockaddr_in structure */
	memcpy(&destSockAddr.sin_addr,
		&destAddr, sizeof(destAddr));
	/* specify the port portion of the address */
	destSockAddr.sin_port = htons(tracker_port);
	/* specify the address family as Internet */
	destSockAddr.sin_family = AF_INET;
	/* create a socket */
	destSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (destSocket == INVALID_SOCKET)
	{
		cerr << "ERROR: socket unsuccessful" << endl;
		status = WSACleanup();
		if (status == SOCKET_ERROR)
			cerr << "ERROR: WSACleanup unsuccessful"
			<< endl;
		return(1);
	}
	cout << "Trying to connect to IP Address: "
		<< tracker_ip << ":" << tracker_port << endl;
}
int connect()
{
	/* connect to the server */
	status = connect(destSocket,
		(LPSOCKADDR)&destSockAddr,
		sizeof(destSockAddr));
	if (status == SOCKET_ERROR)
	{
		cerr << "ERROR: connect unsuccessful" << endl;
		status = closesocket(destSocket);
		if (status == SOCKET_ERROR)
			cerr << "ERROR: closesocket unsuccessful"
			<< endl;
		status = WSACleanup();
		if (status == SOCKET_ERROR)
			cerr << "ERROR: WSACleanup unsuccessful"
			<< endl;
		return(1);
	}
	cout << "Connected..." << endl;
}
long long get_file_size(FILE *fp)
{
	long long size;
	//定义pos
	fpos_t pos;
	//获取文件指针，写入pos
	fgetpos(fp, &pos);
	// 把文件的位置指针移到文件尾
	fseek(fp, 0, SEEK_END);
	// 获取文件长度
	size = ftell(fp);
	printf("该文件的长度为 %1ld 字节\n", size);
	//文件指针还原
	fsetpos(fp, &pos);
	return size;
}
void file_slice()
{
	FILE *fp;
	string path = PATH + string(INPUT_NAME);
	if ((fp = fopen(path.c_str(), "rb+")) == NULL) {  //以二进制读写方式打开
		cerr << "Fail to open file!";
		exit(0);
	}
	//file_size = get_file_size(fp);
	//cout << size;
	//cout << size / CHUNK_COUNT;
	for (int i = 0; i < CHUNK_COUNT; i++)
	{
		//unsigned char buf[100000] = { 0 };
		//unsigned char *buf;
		//buf = (unsigned char*)malloc(sizeof(unsigned char)*size);
		unsigned char *buf = new unsigned char[file_size / CHUNK_COUNT];
		//cout << fread(buf, sizeof(unsigned char), size / CHUNK_COUNT, fp);
		fread(buf, sizeof(unsigned char), file_size / CHUNK_COUNT, fp);
		chunk_list[i].index = i;
		//chunk_list[i].file_chunk = new unsigned char[file_size / CHUNK_COUNT];//改了结构体为unsigned char file_chunk[CHUNK_MAXSIZE];，不需要再初始化分配内存
		memcpy(chunk_list[i].file_chunk, buf, file_size / CHUNK_COUNT);
		//chunk_list[i].file_chunk = buf;//*chunk_list[i].file_chunk = *buf;//典型的复制数组错误示范
		//fseek(fp, size / CHUNK_COUNT, SEEK_CUR);//不需要，fread已经会移动文件指针
	}
	if (file_size % CHUNK_COUNT != 0)
	{
		fseek(fp, -file_size % CHUNK_COUNT, SEEK_END);
		//chunk_list[CHUNK_COUNT].file_chunk = new unsigned char[file_size % CHUNK_COUNT];
		fread(chunk_list[CHUNK_COUNT].file_chunk, sizeof(unsigned char), file_size % CHUNK_COUNT, fp);
	}
	fclose(fp);
}
void save_chunks()
{
	FILE *chunk;
	for (int i = 0; i < CHUNK_COUNT; i++)
	{
		string path = PATH + to_string(i) + ".dat";
		if ((chunk = fopen(path.c_str(), "wb+")) == NULL) {  //以二进制写入/更新方式打开 创建新文件
			cerr << "Fail to open file!";
			exit(0);
		}
		//cout << fwrite(chunk_list[i].file_chunk, sizeof(chunk_list[i].file_chunk), 1, chunk);
		fwrite(chunk_list[i].file_chunk, sizeof(unsigned char), file_size / CHUNK_COUNT, chunk);
		fclose(chunk);
	}
	if (file_size % CHUNK_COUNT != 0)
	{
		if ((chunk = fopen(string(PATH + to_string(CHUNK_COUNT - 1) + ".dat").c_str(), "ab+")) == NULL) {  //以二进制方式打开
			cerr << "Fail to open file!";
			exit(0);
		}
		fwrite(chunk_list[CHUNK_COUNT].file_chunk, sizeof(unsigned char), file_size % CHUNK_COUNT, chunk);
		fclose(chunk);
	}
}
void read_chunks()
{
	FILE *chunk;
	for (int i = 0; i < CHUNK_COUNT; i++)
	{
		string path = SAVE_PATH + to_string(i) + ".dat";
		if ((chunk = fopen(path.c_str(), "rb+")) == NULL) {  //以二进制读写方式打开
			cerr << "Fail to open file!";
			exit(0);
		}

		//unsigned char buf[100000] = { 0 };
		unsigned char *buf = new unsigned char[file_size / CHUNK_COUNT];
		//cout << fread(new_chunk_list[i].file_chunk, sizeof(chunk_list[i].file_chunk), 1, newfp);//没有给file_chunk分配内存所以错误
		//cout << fread(buf, sizeof(chunk_list[i].file_chunk), 1, newfp);//错误的读取或写入方式 会导致每块大小必为4字节
		fread(buf, sizeof(unsigned char), file_size / CHUNK_COUNT, chunk);
		new_chunk_list[i].index = i;
		//new_chunk_list[i].file_chunk = new unsigned char[file_size / CHUNK_COUNT];
		memcpy(new_chunk_list[i].file_chunk, buf, file_size / CHUNK_COUNT);
		fclose(chunk);
	}
	if (file_size % CHUNK_COUNT != 0)
	{
		if ((chunk = fopen(string(SAVE_PATH + to_string(CHUNK_COUNT - 1) + ".dat").c_str(), "rb+")) == NULL) {  //以二进制读写方式打开
			cerr << "Fail to open file!";
			exit(0);
		}
		fseek(chunk, -file_size % CHUNK_COUNT, SEEK_END);
		//new_chunk_list[CHUNK_COUNT].file_chunk = new unsigned char[file_size % CHUNK_COUNT];
		fread(new_chunk_list[CHUNK_COUNT].file_chunk, sizeof(unsigned char), file_size % CHUNK_COUNT, chunk);
		fclose(chunk);
	}
}
void merge_chunks()
{
	FILE *newfp;
	for (int i = 0; i < CHUNK_COUNT; i++)
	{
		string path = SAVE_PATH + string(OUTPUT_NAME);
		if ((newfp = fopen(path.c_str(), "ab+")) == NULL) {  //以二进制追加/更新方式打开
			cerr << "Fail to open file!";
			exit(0);
		}
		fwrite(new_chunk_list[i].file_chunk, sizeof(unsigned char), file_size / CHUNK_COUNT, newfp);
		fclose(newfp);
	}
	if (file_size % CHUNK_COUNT != 0)
	{
		string path = SAVE_PATH + string(OUTPUT_NAME);
		if ((newfp = fopen(path.c_str(), "ab+")) == NULL) {  //以二进制追加/更新方式打开
			cerr << "Fail to open file!";
			exit(0);
		}
		fwrite(new_chunk_list[CHUNK_COUNT].file_chunk, sizeof(unsigned char), file_size % CHUNK_COUNT, newfp);
		fclose(newfp);
	}
}
void send_header()
{
	cout << "Sending Header..." << endl;
	send(clientSocket, to_string(file_size).c_str(), strlen(to_string(file_size).c_str()) + 1, NO_FLAGS_SET);
	Sleep(100);
}
void send_chunks()
{
	cout << "Sending Chunks..." << endl;
	FILE *chunk;
	for (int i = 0; i < CHUNK_COUNT; i++)
	{
		chunk_pack pack;
		string path = PATH + to_string(i) + ".dat";
		if ((chunk = fopen(path.c_str(), "rb+")) == NULL) {  //以二进制读写方式打开
			cerr << "Fail to open file!";
			exit(0);
		}
		unsigned char *buf = new unsigned char[file_size / CHUNK_COUNT];
		fread(buf, sizeof(unsigned char), file_size / CHUNK_COUNT, chunk);
		pack.index = i;
		//pack.file_chunk = new unsigned char[file_size / CHUNK_COUNT];
		memcpy(pack.file_chunk, buf, file_size / CHUNK_COUNT);
		if (file_size % CHUNK_COUNT != 0 && i == CHUNK_COUNT - 1)
		{
			fseek(chunk, -file_size / CHUNK_COUNT, SEEK_CUR);
			//pack.file_chunk = new unsigned char[file_size % CHUNK_COUNT];
			fread(pack.file_chunk, sizeof(unsigned char), file_size / CHUNK_COUNT + file_size % CHUNK_COUNT, chunk);
			fclose(chunk);
		}
		fclose(chunk);
		cout << string("Send chunk ") + to_string(i) + string(" with ") << send(destSocket, (char*)&pack, sizeof(chunk_pack), NO_FLAGS_SET) << "bytes" << endl;
		Sleep(100);
	}
}
void send_single_chunk(int i)
{
	send_header();
	cout << "Sending Chunk..." << endl;
	FILE *chunk;
	chunk_pack pack;
	string path = PATH + to_string(i) + ".dat";
	if ((chunk = fopen(path.c_str(), "rb+")) == NULL) {  //以二进制读写方式打开
		cerr << "Fail to open file!";
		exit(0);
	}
	unsigned char *buf = new unsigned char[file_size / CHUNK_COUNT];
	fread(buf, sizeof(unsigned char), file_size / CHUNK_COUNT, chunk);
	pack.index = i;
	//pack.file_chunk = new unsigned char[file_size / CHUNK_COUNT];
	memcpy(pack.file_chunk, buf, file_size / CHUNK_COUNT);
	if (file_size % CHUNK_COUNT != 0 && i == CHUNK_COUNT - 1)
	{
		fseek(chunk, -file_size / CHUNK_COUNT, SEEK_CUR);
		//pack.file_chunk = new unsigned char[file_size % CHUNK_COUNT];
		fread(pack.file_chunk, sizeof(unsigned char), file_size / CHUNK_COUNT + file_size % CHUNK_COUNT, chunk);
		fclose(chunk);
	}
	fclose(chunk);
	cout << string("Send chunk ") + to_string(i) + string(" with ") << send(clientSocket, (char*)&pack, sizeof(chunk_pack), NO_FLAGS_SET) << "bytes" << endl;
	Sleep(500);
}
void split_ip(char *addr)
{
	sscanf(addr, "%[^:]:%d", peer_ip, &peer_port);
}
void request_tracker(int index)
{
	init_tracker_connection();
	connect();
	char fileindex[100];
	memcpy(fileindex, to_string(index).c_str(), 100);
	char fileaddr[100];
	cout << string("Send request ") + string(" with ") << send(destSocket, (char*)&filename, sizeof(filename), NO_FLAGS_SET) << "bytes" << endl;
	Sleep(1000);
	cout << string("Send request ") + string(" with ") << send(destSocket, (char*)&fileindex, sizeof(fileindex), NO_FLAGS_SET) << "bytes" << endl;
	cout << string("Received response ") + string(" with ") << recv(destSocket, (char*)&fileaddr, sizeof(fileaddr), NO_FLAGS_SET) << "bytes" << endl;
	cout << "fileaddr:" << fileaddr << endl;
	split_ip(fileaddr);
	cout << "peer ip:" << peer_ip << endl;
	cout << "peer port:" << peer_port << endl;
	closesocket(destSocket);
	WSACleanup();
	cout << "close tracker connection"
		<< endl;
}
void torrent_reader()
{
	cout << "Reading Torrent..." << endl;
	FILE *torrent;
	string path = TORRENT_PATH + string("test") + ".torrent";
	if ((torrent = fopen(path.c_str(), "rb")) == NULL) {
		cerr << "Fail to open file!";
		exit(0);
	}
	
	fseek(torrent, 0, SEEK_END);
	long file_size = ftell(torrent);
	fseek(torrent, 0, SEEK_SET);
	
	char* buffer = new char[file_size + 1];
	fread(buffer, 1, file_size, torrent);
	buffer[file_size] = '\0';
	fclose(torrent);
	
	try {
		string torrent_data(buffer, file_size);
		auto torrent_dict = BencodeParser::parse(torrent_data);
		
		if (torrent_dict->type == BencodeType::DICTIONARY) {
			if (torrent_dict->dict_value.find("announce") != torrent_dict->dict_value.end()) {
				string announce_url = torrent_dict->dict_value["announce"]->string_value;
				size_t protocol_end = announce_url.find("://");
				if (protocol_end != string::npos) {
					string rest = announce_url.substr(protocol_end + 3);
					size_t colon_pos = rest.find(':');
					size_t slash_pos = rest.find('/');
					
					if (colon_pos != string::npos && slash_pos != string::npos) {
						strcpy(tracker_ip, rest.substr(0, colon_pos).c_str());
						tracker_port = stoi(rest.substr(colon_pos + 1, slash_pos - colon_pos - 1));
					}
				}
			}
			
			if (torrent_dict->dict_value.find("info") != torrent_dict->dict_value.end()) {
				auto info_dict = torrent_dict->dict_value["info"];
				if (info_dict->dict_value.find("name") != info_dict->dict_value.end()) {
					strcpy(filename, info_dict->dict_value["name"]->string_value.c_str());
				}
				if (info_dict->dict_value.find("length") != info_dict->dict_value.end()) {
					file_size = info_dict->dict_value["length"]->int_value;
				}
			}
		}
	} catch (const std::exception& e) {
		cerr << "Error parsing torrent file: " << e.what() << endl;
		
		fclose(torrent);
		if ((torrent = fopen(path.c_str(), "r")) == NULL) {
			cerr << "Fail to open file!";
			exit(0);
		}
		char buf[100];
		char port_str[100];
		for (int i = 0; fgets(buf, 100, torrent) != NULL;i++) {
			if (i == 0)
			{
				memcpy(filename, buf, 100);
			}
			else if (i == 1)
			{
				memcpy(tracker_ip, buf,100);
			}
			else if (i == 2)
			{
				memcpy(port_str, buf, 100);
			}
			memset(buf, 0, 100);
		}
		char *tmp = NULL;
		if ((tmp = strstr(filename, "\n")))
		{
			*tmp = '\0';
		}
		if ((tmp = strstr(tracker_ip, "\n")))
		{
			*tmp = '\0';
		}
		if ((tmp = strstr(port_str, "\n")))
		{
			*tmp = '\0';
		}
		stringstream strIn;
		strIn << port_str;
		strIn >> tracker_port;
	}
	
	delete[] buffer;
	cout<< "filename:" << filename << endl;
	cout << "tracker_ip:" << tracker_ip << endl;
	cout << "tracker_port:" << tracker_port << endl;
}
void recv_header()
{
	recv(destSocket, buffer, MAXBUFLEN, NO_FLAGS_SET);
	stringstream strIn;
	strIn << buffer;
	strIn >> file_size;
	cout << "header:" << file_size << endl;
	memset(buffer, 0, MAXBUFLEN);
}
void recv_chunks()
{
	FILE *chunk;
	for (int i = 0; i < CHUNK_COUNT; i++)
	{
		chunk_pack pack;
		memset(buffer, 0, MAXBUFLEN);
		cout << string("Received chunk ") + to_string(i) + string(" with ") << recv(clientSocket, buffer, sizeof(buffer), NO_FLAGS_SET) << "bytes" << endl;
		memcpy(&pack, buffer, sizeof(buffer));
		//cout << pack.index << endl;
		//cout << pack.file_chunk << endl;
		string path = PATH + to_string(pack.index) + ".dat";
		if ((chunk = fopen(path.c_str(), "wb+")) == NULL) {  //以二进制写入/更新方式打开 创建新文件
			cerr << "Fail to open file!";
			exit(0);
		}
		fwrite(pack.file_chunk, sizeof(unsigned char), file_size / CHUNK_COUNT, chunk);
		fclose(chunk);
		if (file_size % CHUNK_COUNT != 0 && i == CHUNK_COUNT - 1)
		{
			if ((chunk = fopen(path.c_str(), "ab+")) == NULL) {  //以二进制追加/更新方式打开
				cerr << "Fail to open file!";
				exit(0);
			}
			fseek(chunk, -file_size % CHUNK_COUNT, SEEK_END);
			fwrite(pack.file_chunk, sizeof(unsigned char), file_size % CHUNK_COUNT, chunk);
			fclose(chunk);
		}
		fclose(chunk);
	}
}
int recv_single_chunk()
{
	FILE *chunk;
	chunk_pack pack;
	memset(buffer, 0, MAXBUFLEN);
	int res = recv(destSocket, (char *)buffer, sizeof(buffer), NO_FLAGS_SET);
	memcpy(&pack, buffer, sizeof(buffer));
	cout << string("Received chunk ") + to_string(pack.index) + string(" with ") << res << "bytes" << endl;
	//cout << pack.index << endl;
	//cout << pack.file_chunk << endl;
	string path = SAVE_PATH + to_string(pack.index) + ".dat";
	if ((chunk = fopen(path.c_str(), "wb+")) == NULL) {  //以二进制写入/更新方式打开 创建新文件
		cerr << "Fail to open file!";
		exit(0);
	}
	fwrite(pack.file_chunk, sizeof(unsigned char), file_size / CHUNK_COUNT, chunk);
	fclose(chunk);
	if (file_size % CHUNK_COUNT != 0 && pack.index == CHUNK_COUNT - 1)
	{
		if ((chunk = fopen(path.c_str(), "ab+")) == NULL) {  //以二进制追加/更新方式打开
			cerr << "Fail to open file!";
			exit(0);
		}
		fseek(chunk, -file_size % CHUNK_COUNT, SEEK_END);
		fwrite(pack.file_chunk, sizeof(unsigned char), file_size % CHUNK_COUNT, chunk);
		fclose(chunk);
	}
	fclose(chunk);
	return res;
}
void loop_mode_recv()
{
	recv_header();
	int cnt = 0;
	while (cnt < CHUNK_COUNT)
	{
		numrcv = recv_single_chunk();
		if (numrcv > 0)
		{
			cnt++;
		}
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
			init_server();
		}
		cout << buffer << endl;
	} /* while */
	read_chunks();
	merge_chunks();
}
void init_server()
{
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
void request_single_chunk(int index)
{
	init_peer_connection();
	connect();
	
	send_bt_handshake();
	
	char handshake_response[68];
	recv(destSocket, handshake_response, 68, NO_FLAGS_SET);
	
	send_bt_interested();
	
	send_bt_request(index, 0, file_size / CHUNK_COUNT);
	
	recv_header();
	recv_single_chunk();
	Sleep(100);
	closesocket(destSocket);
	WSACleanup();
	cout << "close request" << endl;
}
void recv_chunk_request()
{
	cout << "Receiving chunk requests..." << endl;
	recv_bt_handshake();
	
	while (1)
	{
		cout << "Receiving..." << endl;
		recv_bt_message();
		
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
			init_server();
		}
	}
}

void send_bt_handshake()
{
	std::string handshake = BTProtocol::create_handshake("", "PEER_ID_12345678901");
	send(destSocket, handshake.c_str(), handshake.length(), NO_FLAGS_SET);
	cout << "Sent BT handshake" << endl;
}

void recv_bt_handshake()
{
	char handshake[68];
	numrcv = recv(clientSocket, handshake, 68, NO_FLAGS_SET);
	if (BTProtocol::validate_handshake(handshake, numrcv)) {
		cout << "Received valid BT handshake" << endl;
		std::string response = BTProtocol::create_handshake("", "PEER_ID_SERVER_01");
		send(clientSocket, response.c_str(), response.length(), NO_FLAGS_SET);
	} else {
		cout << "Invalid handshake received" << endl;
	}
}

void send_bt_message(unsigned char msg_id, const char* payload, int payload_len)
{
	std::string message = BTProtocol::create_message(msg_id, payload, payload_len);
	send(clientSocket, message.c_str(), message.length(), NO_FLAGS_SET);
}

void recv_bt_message()
{
	char msg_buffer[1024];
	numrcv = recv(clientSocket, msg_buffer, sizeof(msg_buffer), NO_FLAGS_SET);
	if (numrcv <= 0) return;
	
	try {
		BTMessage msg = BTProtocol::parse_message(msg_buffer, numrcv);
		
		if (msg.id == 255) {
			cout << "Received keep-alive message" << endl;
			return;
		}
		
		cout << "Received BT message ID: " << (int)msg.id << endl;
		
		switch (msg.id) {
			case BT_MSG_CHOKE:
				cout << "Received choke" << endl;
				break;
			case BT_MSG_UNCHOKE:
				cout << "Received unchoke" << endl;
				break;
			case BT_MSG_INTERESTED:
				cout << "Received interested" << endl;
				send_bt_message(BT_MSG_UNCHOKE, nullptr, 0);
				break;
			case BT_MSG_NOT_INTERESTED:
				cout << "Received not interested" << endl;
				break;
			case BT_MSG_REQUEST:
				if (msg.length >= 13) {
					int piece_index = (msg.payload[0] << 24) | (msg.payload[1] << 16) | (msg.payload[2] << 8) | msg.payload[3];
					int block_offset = (msg.payload[4] << 24) | (msg.payload[5] << 16) | (msg.payload[6] << 8) | msg.payload[7];
					int block_length = (msg.payload[8] << 24) | (msg.payload[9] << 16) | (msg.payload[10] << 8) | msg.payload[11];
					cout << "Received request for piece " << piece_index << " offset " << block_offset << " length " << block_length << endl;
					send_single_chunk(piece_index);
				}
				break;
		}
		
		if (msg.payload) {
			delete[] msg.payload;
		}
	} catch (const std::exception& e) {
		cout << "Error parsing BT message: " << e.what() << endl;
	}
}

void send_bt_interested()
{
	send_bt_message(BT_MSG_INTERESTED, nullptr, 0);
	cout << "Sent interested message" << endl;
}

void send_bt_request(int piece_index, int block_offset, int block_length)
{
	char payload[12];
	payload[0] = (piece_index >> 24) & 0xFF;
	payload[1] = (piece_index >> 16) & 0xFF;
	payload[2] = (piece_index >> 8) & 0xFF;
	payload[3] = piece_index & 0xFF;
	
	payload[4] = (block_offset >> 24) & 0xFF;
	payload[5] = (block_offset >> 16) & 0xFF;
	payload[6] = (block_offset >> 8) & 0xFF;
	payload[7] = block_offset & 0xFF;
	
	payload[8] = (block_length >> 24) & 0xFF;
	payload[9] = (block_length >> 16) & 0xFF;
	payload[10] = (block_length >> 8) & 0xFF;
	payload[11] = block_length & 0xFF;
	
	send_bt_message(BT_MSG_REQUEST, payload, 12);
	cout << "Sent request for piece " << piece_index << endl;
}
