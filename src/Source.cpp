#define WIN32_LEAN_AND_MEAN  
#define _WINSOCK_DEPRECATED_NO_WARNINGS // for conflict by winsock

#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <vector>
#include <tchar.h>
#include <stdlib.h>
#include <string>

#define sHELLO "SOCKET CONNECTED\r\n"
#define SN_TIMER  1000
#define SN_CLASS  _T("SNAKE")
#define SN_APP    _T("Змейка")
#define SN_WIDTH  640
#define SN_HEIGHT 480
#define SN_SIZE   16
#define SN_NONE   0
#define SN_BODY   1
#define SN_FOOD   2
#define SN_HEAD   3
#define COLS      40
#define ROWS      30
int  create_window(HINSTANCE hinst, LPCTSTR cap);
void draw(HDC mdc, int dir, HBRUSH back, HPEN pen, HBRUSH food);
void draw_text(HDC mdc);
void start(int& dir);
void pos_food(void);
BYTE g_mat[ROWS][COLS];
bool g_over;
// variables of server
bool serverUp;
//1-close 8-up 2-down 4-left 6-right
int  ClientP;
//           function of server
DWORD WINAPI CreateServer(LPVOID);
DWORD WINAPI SxToClient(LPVOID);
//
std::vector<std::string> MessageVector;	//vestor for server's messages

///////////////////////////////////////////////////////////////////////

struct point {
	int x, y;
	point() :x(0), y(0) {}
	point(int _x, int _y) :x(_x), y(_y) {}
};
std::vector<point> g_snake;

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPTSTR, int) {
	return  create_window(hinst, SN_APP);
}

void start(int& dir) {
	serverUp = false;
	ClientP = 8;
	dir = VK_UP;
	g_snake.clear();
	for (int i = 0; i < ROWS; ++i) {
		for (int j = 0; j < COLS; ++j)
			g_mat[i][j] = SN_NONE;
	}

	for (int b = 0; b < 3; ++b) {
		g_snake.push_back(point(COLS / 2, ROWS / 2 + b));
		const point& p = g_snake.back();
		g_mat[p.y][p.x] = SN_BODY;
	}
	pos_food();
	g_over = false;
}

void draw(HDC mdc, int dir, HBRUSH back, HPEN pen, HBRUSH food) {
	if (g_over) {
		draw_text(mdc);
		return;
	}

	point prev = g_snake.front();
	switch (serverUp) {
	case false: {
		switch (dir) {
		case VK_LEFT:
			--g_snake.front().x;
			break;
		case VK_RIGHT:
			++g_snake.front().x;
			break;
		case VK_UP:
			--g_snake.front().y;
			break;
		case VK_DOWN:
			++g_snake.front().y;
			break;
		}
	}
	break;
	case true: {
		switch (ClientP) {
		case 4:
			--g_snake.front().x;
			break;
		case 6:
			++g_snake.front().x;
			break;
		case 8:
			--g_snake.front().y;
			break;
		case 2:
			++g_snake.front().y;
			break;
		case 1:
			serverUp = false;
			ClientP = 6;
			break;
		}
	}
	break;
	}
	//colision
	point q = g_snake.front();

	if (q.x < 0) g_snake.front().x = COLS-1;
	if (q.y < 0) g_snake.front().y = ROWS-1;
	if (q.y >= ROWS) g_snake.front().y = 0;
	if (q.x >= COLS) g_snake.front().x = 0;
	//if ((q.x < 0) || (q.y < 0) || (q.y >= ROWS) || (q.x >= COLS) || (g_mat[q.y][q.x] == SN_BODY)) {
	//	g_over = true;
	//	return;
	//}

	point e = g_snake.back();
	std::vector<point>::size_type s;
	for (s = 1; s < g_snake.size(); ++s)
		std::swap(g_snake[s], prev);

	if (g_mat[q.y][q.x] == (BYTE)SN_FOOD) {
		if (g_snake.size() > 512) {
			g_over = true;
			return;
		}
		g_snake.push_back(e);
		g_mat[q.y][q.x] = SN_NONE;
		pos_food();
	}
	g_mat[e.y][e.x] = SN_NONE;
	g_mat[g_snake.front().y][g_snake.front().x] = SN_HEAD;

	for (s = 1; s < g_snake.size(); ++s)
		g_mat[g_snake[s].y][g_snake[s].x] = SN_BODY;

	int     x, y, m;
	RECT    rc;
	HGDIOBJ a, b;
	a = SelectObject(mdc, back);
	b = SelectObject(mdc, pen);
	for (int i = 0; i < ROWS; ++i) {
		for (int j = 0; j < COLS; ++j) {
			switch (g_mat[i][j]) {
			case SN_HEAD:
				x = j * SN_SIZE;
				y = i * SN_SIZE;
				m = SetROP2(mdc, R2_XORPEN);
				Ellipse(mdc, x, y, x + SN_SIZE, y + SN_SIZE);
				SetROP2(mdc, m);
				break;
			case SN_BODY: 
				x = j * SN_SIZE;
				y = i * SN_SIZE;
				RoundRect(mdc, x, y, x + SN_SIZE, y + SN_SIZE, 14, 14);
				break;
			case SN_FOOD:
				x = j * SN_SIZE;
				y = i * SN_SIZE;
				SetRect(&rc, x, y, x + SN_SIZE, y + SN_SIZE);
				FillRect(mdc, &rc, food);
				break;
			}
		}
	}
	SelectObject(mdc, a);
	SelectObject(mdc, b);
}

void pos_food(void) {
	int x, y;
	do {
		x = rand() % COLS;
		y = rand() % ROWS;
		if (g_mat[y][x] == SN_NONE) {
			g_mat[y][x] = SN_FOOD;
			break;
		}
	} while (1);
}


void draw_text(HDC mdc) {
	const TCHAR s1[] = _T("КОНЕЦ ИГРЫ");
	const int   n1 = sizeof(s1) / sizeof(s1[0]) - 1;
	SIZE sz;
	GetTextExtentPoint32(mdc, s1, n1, &sz);
	TextOut(mdc, (SN_WIDTH - sz.cx) / 2, (SN_HEIGHT - sz.cy) / 2 - sz.cy * 2, s1, n1);

	const TCHAR s2[] = _T("НАЧАТЬ ЗАНОВО КЛАВИША ENTER");
	const int   n2 = sizeof(s2) / sizeof(s2[0]) - 1;
	GetTextExtentPoint32(mdc, s2, n2, &sz);
	TextOut(mdc, (SN_WIDTH - sz.cx) / 2, (SN_HEIGHT - sz.cy) / 2 + sz.cy, s2, n2);
}


LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBITMAP hbm = NULL;
	static HBRUSH back = NULL;
	static HPEN pen = NULL;
	static HBRUSH food = NULL;
	static HDC mdc = NULL;
	static int dir = VK_UP;
	HDC hdc;

	switch (msg) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		mdc = CreateCompatibleDC(hdc);
		hbm = CreateCompatibleBitmap(hdc, SN_WIDTH, SN_HEIGHT);
		SelectObject(mdc, hbm);
		ReleaseDC(hwnd, hdc);																				
		back = CreateSolidBrush(RGB(0x55, 0xFF, 0x55));														
		pen = CreatePen(PS_SOLID, 1, RGB(0, 0xAA, 0));														
		food = CreateSolidBrush(RGB(0xFF, 0x88, 0x35));														
		start(dir);																							
		//
		CreateThread(NULL, NULL, CreateServer, NULL, NULL, NULL);
		//
		SetTimer(hwnd, SN_TIMER, 95, NULL);																	
		break;																								
	case WM_ERASEBKGND:																						
		PatBlt(mdc, 0, 0, SN_WIDTH, SN_HEIGHT, WHITENESS);		

		draw(mdc, dir, back, pen, food);	

		BitBlt((HDC)wParam, 0, 0, SN_WIDTH, SN_HEIGHT, mdc, 0, 0, SRCCOPY);		
		hdc = GetDC(hwnd);
		for (int i = 0; i < MessageVector.size(); i++)
			TextOutA(hdc, 10, 10, MessageVector[i].c_str(), MessageVector[i].size());
		ReleaseDC(hwnd, hdc);
		return 1;		

	case WM_KEYDOWN:																						
		switch (LOWORD(wParam)) {																			
		case VK_LEFT:																						
			dir = VK_LEFT;																					
			break;																							
		case VK_RIGHT:																						
			dir = VK_RIGHT;																					
			break;																							
		case VK_UP:																							
			dir = VK_UP;																					
			break;																							
		case VK_DOWN:																						
			dir = VK_DOWN;																					
			break;																							
		case VK_RETURN:																						
			if (g_over)																						
				start(dir);
			break;
		}
		break;
	
	case WM_TIMER:
		{
		
		}
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_DESTROY:
		DeleteDC(mdc);
		DeleteObject(hbm);
		DeleteObject(back);
		DeleteObject(pen);
		DeleteObject(food);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int create_window(HINSTANCE hinst, LPCTSTR cap) {
	WNDCLASSEX cls = { 0 };
	cls.cbSize = sizeof(cls);
	cls.lpfnWndProc = (WNDPROC)wnd_proc;
	cls.hInstance = hinst;
	cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	cls.lpszClassName = SN_CLASS;
	if (!RegisterClassEx(&cls))
		return 1;

	DWORD sty = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
	RECT  rc = { 0, 0, SN_WIDTH, SN_HEIGHT };
	AdjustWindowRectEx(&rc, sty, FALSE, 0);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	HWND hwnd = CreateWindowEx(0, SN_CLASS, cap, sty, (GetSystemMetrics(SM_CXSCREEN) - width) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - height) / 2, width, height, NULL, NULL, hinst, NULL);
	if (hwnd == NULL) {
		UnregisterClass(SN_CLASS, hinst);
		return 1;
	}
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterClass(SN_CLASS, hinst);
	return 0;
}

//Creating Server
DWORD WINAPI
CreateServer(LPVOID)
{

	// never mind bufer
	char buff[1024];
	// init the socket lib
	// WSADATA - 400 bytes
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// error message!
		MessageVector.push_back("Error WSAStartup " + WSAGetLastError());
		return-1;
	}
	// create the socket
	SOCKET mysocket;
	// AF_INET - socket of internet
	// SOCK_STREAM - socket of stream (with set connecting)
	// 0 - default TCP protocol
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		MessageVector.push_back("Error socket " + WSAGetLastError());
		WSACleanup(); // deinitialization Winsock lib
		return-1;
	}
	// soket's binding with local host
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(666); // network queue
	local_addr.sin_addr.s_addr = 0;
	// call bind() for biding
	if (bind(mysocket, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		MessageVector.push_back("Error bind %d\n" + WSAGetLastError());
		closesocket(mysocket); // socet's close
		WSACleanup();
		return -1;
	}
	// size of queue - 0x100  
	// waiting connect
	if (listen(mysocket, 0x100))
	{
		MessageVector.push_back("Error listen " + WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}

	//waiting message
	MessageVector.push_back("waiting connect ...");

	// extract message of queue 
	SOCKET client_socket; // socket for client
	sockaddr_in client_addr; // client's address (setting by system)


	int client_addr_size = sizeof(client_addr);

	// цикл извлечения запросов на подключение из очереди
	// функции accept необходимо передать размер структуры 
	while ((client_socket = accept(mysocket, (sockaddr *)&client_addr, \
		&client_addr_size)))
	{
		// take the host's name
		HOSTENT *hst;
		hst = gethostbyaddr((char *)&client_addr.sin_addr.s_addr, 4, AF_INET);
		MessageVector.push_back("new connect "+ (std::string)hst->h_name+ (std::string)" "+ inet_ntoa(client_addr.sin_addr));
		DWORD thID;
		// new thread for client
		CreateThread(NULL, NULL, SxToClient, &client_socket, NULL, &thID);

	}
	return 0;
}

DWORD WINAPI SxToClient(LPVOID _client_socket)
{
	SOCKET my_sock;
	my_sock = ((SOCKET *)_client_socket)[0];
	// send hello message
	send(my_sock, sHELLO, sizeof(sHELLO), 0);
	serverUp = true;
	int bytes_recv;

	const int max_client_buffer_size = 1;
	char buf[max_client_buffer_size];
	while (atoi(buf) != 1) 
	{
		bytes_recv = recv(my_sock, buf, max_client_buffer_size, 0);
		
		if (bytes_recv != SOCKET_ERROR ) {
			if(atoi(buf) == 1 || atoi(buf) == 2 || atoi(buf) == 8 || atoi(buf) == 4 || atoi(buf) == 6)
				ClientP = atoi(buf);
		}
	}

	// close socket
	MessageVector.push_back("disconnect ");
	closesocket(my_sock);
	return 0;
}