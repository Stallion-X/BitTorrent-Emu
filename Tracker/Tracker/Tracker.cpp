#include <windows.h>
#include <iostream>
#include <winsock.h>
#include <string>
#include <sstream>
#include <map>
#include "../../bencode.h"
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
void handle_announce_request(const std::string& request);
void handle_scrape_request(const std::string& request);
std::string url_decode(const std::string& str);

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
		
		string request(buffer);
		if (request.find("GET /announce") == 0) {
			handle_announce_request(request);
		} else if (request.find("GET /scrape") == 0) {
			handle_scrape_request(request);
		} else {
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
		}
	}
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
	if ((torrent = fopen(path.c_str(), "wb+")) == NULL) {
		cerr << "Fail to open file!";
		exit(0);
	}
	
	auto torrent_dict = BencodeValue::create_dictionary();
	
	string announce_url = "http://" + string(PUBLIC_IP) + ":" + to_string(PORT) + "/announce";
	torrent_dict->dict_value["announce"] = BencodeValue::create_string(announce_url);
	
	auto info_dict = BencodeValue::create_dictionary();
	info_dict->dict_value["name"] = BencodeValue::create_string(file_list[index].name);
	info_dict->dict_value["length"] = BencodeValue::create_integer(file_list[index].file_size);
	info_dict->dict_value["piece length"] = BencodeValue::create_integer(file_list[index].file_size / file_list[index].chunk_count);
	
	string pieces_hash = "";
	for (int i = 0; i < file_list[index].chunk_count; i++) {
		pieces_hash += "01234567890123456789";
	}
	info_dict->dict_value["pieces"] = BencodeValue::create_string(pieces_hash);
	
	torrent_dict->dict_value["info"] = info_dict;
	
	string encoded = BencodeParser::encode(torrent_dict);
	fwrite(encoded.c_str(), 1, encoded.length(), torrent);
	fclose(torrent);
}

void handle_announce_request(const std::string& request)
{
	cout << "Handling announce request" << endl;
	
	auto response_dict = BencodeValue::create_dictionary();
	response_dict->dict_value["interval"] = BencodeValue::create_integer(1800);
	response_dict->dict_value["complete"] = BencodeValue::create_integer(3);
	response_dict->dict_value["incomplete"] = BencodeValue::create_integer(1);
	
	auto peers_list = BencodeValue::create_list();
	for (int i = 0; i < FILELIST_MAXSIZE && strlen(file_list[i].name) > 0; i++) {
		for (const auto& chunk_pair : file_list[i].chunk_addr) {
			auto peer_dict = BencodeValue::create_dictionary();
			string addr = chunk_pair.second;
			size_t colon_pos = addr.find(':');
			if (colon_pos != string::npos) {
				peer_dict->dict_value["ip"] = BencodeValue::create_string(addr.substr(0, colon_pos));
				peer_dict->dict_value["port"] = BencodeValue::create_integer(stoi(addr.substr(colon_pos + 1)));
				peer_dict->dict_value["peer id"] = BencodeValue::create_string("peer" + to_string(i));
				peers_list->list_value.push_back(peer_dict);
				break;
			}
		}
	}
	response_dict->dict_value["peers"] = peers_list;
	
	string response_data = BencodeParser::encode(response_dict);
	string http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + 
		to_string(response_data.length()) + "\r\n\r\n" + response_data;
	
	send(clientSocket, http_response.c_str(), http_response.length(), NO_FLAGS_SET);
}

void handle_scrape_request(const std::string& request)
{
	cout << "Handling scrape request" << endl;
	
	auto response_dict = BencodeValue::create_dictionary();
	auto files_dict = BencodeValue::create_dictionary();
	
	for (int i = 0; i < FILELIST_MAXSIZE && strlen(file_list[i].name) > 0; i++) {
		auto file_stats = BencodeValue::create_dictionary();
		file_stats->dict_value["complete"] = BencodeValue::create_integer(3);
		file_stats->dict_value["incomplete"] = BencodeValue::create_integer(1);
		file_stats->dict_value["downloaded"] = BencodeValue::create_integer(10);
		files_dict->dict_value[string(file_list[i].name)] = file_stats;
	}
	
	response_dict->dict_value["files"] = files_dict;
	
	string response_data = BencodeParser::encode(response_dict);
	string http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + 
		to_string(response_data.length()) + "\r\n\r\n" + response_data;
	
	send(clientSocket, http_response.c_str(), http_response.length(), NO_FLAGS_SET);
}

std::string url_decode(const std::string& str)
{
	std::string result;
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%' && i + 2 < str.length()) {
			int hex_val;
			std::istringstream hex_stream(str.substr(i + 1, 2));
			hex_stream >> std::hex >> hex_val;
			result += static_cast<char>(hex_val);
			i += 2;
		} else if (str[i] == '+') {
			result += ' ';
		} else {
			result += str[i];
		}
	}
	return result;
}
