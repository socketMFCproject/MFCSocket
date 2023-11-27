// CServerChat.cpp: 구현 파일
//

#include "pch.h"
#include "SocketChat_improve.h"
#include "afxdialogex.h"
#include "CServerChat.h"
//#include "Common.h"
//#include "Dol_Check.h"
#include <thread>
#include "CGameover.h"

#define SERVERPORT 9000
#define BUFSIZE    512
#define cell_size 40

#define M_SERVER_RECV_UPDATE (WM_USER + 10) // 서버에서 사용할 사용자 정의 메시지
// CServerChat 대화 상자

//HANDLE hServerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE hClientEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

bool isServerTurn = FALSE;

IMPLEMENT_DYNAMIC(CServerChat, CDialogEx)

CServerChat::CServerChat(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_CHAT_DIALOG, pParent)
	, m_serverMsg(_T(""))
{

}

CServerChat::~CServerChat()
{
	// 모든 클라이언트 소켓을 닫습니다.
	for (auto socket : m_clientSockets)
	{
		shutdown(socket, SD_BOTH); // 더 이상의 송수신 비활성화
		closesocket(socket);
	}

	// 윈속 정리
	WSACleanup();
}

void CServerChat::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVER_CHAT_LIST, m_chatList);
	DDX_Control(pDX, IDC_LIST1, m_orderList);
	DDX_Text(pDX, IDC_SERVER_MSG, m_serverMsg);
	DDX_Control(pDX, IDC_SERVER_SEND_BUTTON, m_sendButton);
}


BEGIN_MESSAGE_MAP(CServerChat, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_SERVER_SEND_BUTTON, &CServerChat::OnBnClickedServerSendButton)
	ON_MESSAGE(M_SERVER_RECV_UPDATE, OnUpdateServerChat)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CServerChat::OnPaint()
{
	if (IsIconic())//내 프로그램이 아이콘화되었는지 확인
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
	}
	else
	{
		CPaintDC dc(this);//여기서만 CClientDC사용, Flag성변수를 1->0으로만드는 기능 포함되어있음
		CBrush brownBrush(RGB(236, 187, 90)); // 갈색 브러시 생성
		dc.SelectObject(&brownBrush); // 갈색 브러시 선택
		dc.Rectangle(cell_size - 10, cell_size - 10, cell_size*13 + 10, cell_size*13 + 10);
		for (int y = 0; y < 12; y++)
		{
			for (int x = 0; x < 12; x++)
			{
				dc.Rectangle(cell_size + x * cell_size, cell_size + y * cell_size,
					cell_size + x * cell_size + cell_size + 1, cell_size + y * cell_size + cell_size + 1);
			}
		}

		int radius = 5;
		CPen blackPen(PS_SOLID, 1, RGB(0, 0, 0)); // 검은색 펜 생성
		CPen* oldPen = dc.SelectObject(&blackPen); // 현재 펜 저장

		for (int y = 0; y <= 12; y++)
		{
			for (int x = 0; x <= 12; x++)
			{
				if (y % 3 == 0 && x % 3 == 0) {
					CBrush blackBrush(RGB(0, 0, 0)); // 검은색 브러시 생성
					CBrush* oldBrush = dc.SelectObject(&blackBrush); // 현재 브러시 저장

					dc.Ellipse(cell_size + x * cell_size - radius, cell_size + y * cell_size - radius, cell_size + x * cell_size + radius, cell_size + y * cell_size + radius);

					dc.SelectObject(oldBrush); // 이전 브러시로 복원
				}
			}
		}

		dc.SelectObject(oldPen); // 이전 펜으로 복원

		CBrush* p_old_brush = (CBrush*)dc.SelectStockObject(BLACK_BRUSH);
		for (int y = 1; y <= 13; y++)//저장된 바둑돌 재생성
		{
			for (int x = 1; x <= 13; x++)
			{
				if (m_dol[y - 1][x - 1] > 0)
				{

					if (m_dol[y - 1][x - 1] == 1)
					{
						dc.SelectStockObject(BLACK_BRUSH);
					}
					else
					{
						dc.SelectStockObject(WHITE_BRUSH);
					}

					dc.Ellipse(x * cell_size - 20, y * cell_size - 20, x * cell_size + 20, y * cell_size + 20);

				}
			}
		}
		dc.SelectObject(p_old_brush);
		//CDialogEx::OnPaint();
	}
}


LRESULT CServerChat::OnUpdateServerChat(WPARAM wParam, LPARAM lParam) {
	CString* pStr = reinterpret_cast<CString*>(lParam);
	m_chatList.AddString(pStr->GetString());
	delete pStr; // 동적으로 할당된 CString 객체를 삭제
	return 0;
}


UINT CServerChat::AcceptThread(LPVOID pParam)
{
    CServerChat* pThis = reinterpret_cast<CServerChat*>(pParam);
    SOCKET listenSocket = pThis->m_listenSocket;
    sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);

    while (true)
    {
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientaddr, &addrlen);
        if (clientSocket == INVALID_SOCKET) continue;
		AfxMessageBox(_T("클라이언트 연결 성공"));

        //std::lock_guard<std::mutex> lock(pThis->m_csClientSockets);
        pThis->m_clientSockets.push_back(clientSocket);
		
        //AfxBeginThread(ClientThread, (LPVOID)clientSocket);
		char buf[BUFSIZE+1];
		int len;

		
		CString portNum;
		portNum.Format(_T("%d"), SERVERPORT);
		CStringA strAnsi(portNum); // 유니코드 CString을 ANSI 문자열로 변환
		len = strAnsi.GetLength() + 1;

		// 버퍼 크기를 확인하고 널 종료 문자를 포함하여 복사
		buf[0] = 3;
		strncpy_s(buf + 1, BUFSIZE, strAnsi, len);
		

		// 데이터 보내기(고정 길이)
		int retval = send(clientSocket, (char*)&len, sizeof(int), 0);
		// 데이터 보내기(가변 길이)
		retval = send(clientSocket, buf, len, 0);
		while (true)
		{
			int retval = recv(clientSocket, (char*)&len, sizeof(int), 0);
			if (retval <= 0) break;
			if (len >= BUFSIZE) break;

			retval = recv(clientSocket, buf, len, 0);
			if (retval <= 0) break;

			buf[retval] = '\0';

			if (buf[0] == 1) {
			// 동적 할당을 사용하지 않고 스택에 CString 객체를 생성
			CString str(buf + 1);
			
			// 메시지를 UI 스레드로 전달
			pThis->PostMessage(M_SERVER_RECV_UPDATE, 0, (LPARAM)new CString(str));

			}
			else if (buf[0] == 2) {

				//SetEvent(hClientEvent);
				isServerTurn = TRUE;

				//오목 x y 좌표
				int x = buf[1];
				int y = buf[2];
				pThis->recivePoint(x, y, pParam);
				CString str;
				str.Format(_T("x : %d y: %d"), x, y);
				pThis->PostMessage(M_SERVER_RECV_UPDATE, 0, (LPARAM)new CString(str));


			}
			else if (buf[0] == 3) {
				//상대방 포트 번호 받기
				CString str(buf + 1);
				pThis->PostMessage(M_SERVER_RECV_UPDATE, 0, (LPARAM)new CString(str));
			}

		
			
		}

		// 소켓 닫기s
		closesocket(clientSocket);

    }
    return 0;
}

void CServerChat::recivePoint(int x, int y, LPVOID pParam) {
	CServerChat* pThis = reinterpret_cast<CServerChat*>(pParam);
	CClientDC dc(this);
	int m_dol_state_ = (pThis->m_dol_state == 1 ? 0 : 1);
	pThis->m_dol[y - 1][x - 1] = m_dol_state_ + 1;
	printf("%d %d", x, y);

	int rx = x;
	int ry = y;
	if (x > 0 && x <= 13 && y > 0 && y <= 13) {
		x *= 40;
		y *= 40;

		CBrush* p_old_brush;
		//client 백돌 수신
		p_old_brush = (CBrush*)dc.SelectStockObject(WHITE_BRUSH);

		dc.Ellipse(x - 20, y - 20, x + 20, y + 20);
		dc.SelectObject(p_old_brush);
		if (CheckWin(rx - 1, ry - 1, m_dol_state_)) {
			// 게임 이겼을 경우
			//일단 이기면 좌표 창에 우승자 표시
			CString winString;
			winString = _T("Winner is ");
			m_orderList.AddString(winString + (!m_dol_state_ ? "black" : "white"));


			//게임 종료 화면 띄우기
			CGameover gameover;
			gameover.SetWinner(!m_dol_state_ ? 1 : 2); // 흑돌이 이겼으면 1, 백돌이 이겼으면 2
			gameover.DoModal();
		}

	}
}


BOOL CServerChat::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0);

	// 소켓 생성
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	//if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;

	
	

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(m_listenSocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	//if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(m_listenSocket, SOMAXCONN);
	//if (retval == SOCKET_ERROR) err_quit("listen()");

	AfxBeginThread(AcceptThread, this);

	//SetEvent(hClientEvent);
	isServerTurn = TRUE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CServerChat::OnBnClickedServerSendButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE+1];
	int len;

	// 서버와 데이터 통신
	CString msg = m_serverMsg;
	CStringA strAnsi(msg); // 유니코드 CString을 ANSI 문자열로 변환
	len = strAnsi.GetLength() + 1;

	// 버퍼 크기를 확인하고 널 종료 문자를 포함하여 복사
	buf[0] = 1;
	strncpy_s(buf+1, BUFSIZE, strAnsi, len);
	SOCKET clientSocket = m_clientSockets[0];

	// 데이터 보내기(고정 길이)
	int retval = send(clientSocket, (char*)&len, sizeof(int), 0);
	// 데이터 보내기(가변 길이)
	retval = send(clientSocket, buf, len, 0);

	m_chatList.AddString(msg);

}
//
//오목 규칙 확인
//
void CServerChat::OnSendPosition(int x, int y) {
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	int len = 4;


	// 버퍼 크기를 확인하고 널 종료 문자를 포함하여 복사
	buf[0] = 2;
	buf[1] = x;
	buf[2] = y;

	SOCKET clientSocket = m_clientSockets[0];

	// 데이터 보내기(고정 길이)
	int retval = send(clientSocket, (char*)&len, sizeof(int), 0);
	// 데이터 보내기(가변 길이)
	retval = send(clientSocket, buf, len, 0);
}



//void CServerChat::SavePosition(int x, int y) {
//	m_dol[y - 1][x - 1] = m_dol_state + 1;
//	if (CheckWin(x - 1, y - 1)) {
//		// 게임 이겼을 경우
//		//일단 이기면 좌표 창에 우승자 표시
//		CString winString;
//		winString = _T("Winner is ");
//		m_orderList.AddString(winString +(!m_dol_state ? "black" : "white"));
//
//		
//		//게임 종료 화면 띄우기
//		CGameover gameover;
//		gameover.SetWinner(!m_dol_state ? 1 : 2); // 흑돌이 이겼으면 1, 백돌이 이겼으면 2
//		gameover.DoModal();
//
//	}
//	return;
//}
//헤더로 뺼 것들 
bool CServerChat::CheckFive(int x, int y, int dx, int dy, int m_dol_state_) {
	int count = 0;
	int dx_ = dx;
	int dy_ = dy;
	bool che = false;
	for (int i = 0; i < 5; ++i) {
		int newX = x + i * dx_;
		int newY = y + i * dy_;
		if (newX < 0 || newX >= 14 || newY < 0 || newY >= 14)
			break;
		if (m_dol[newY][newX] != m_dol_state_ + 1)
			break;
		count += 1;
	}
	if (count >= 5) {
		return true;
	}
	dx_ *= -1;
	dy_ *= -1;
	for (int i = 1; i < 5; ++i) {
		int newX = x + i * dx_;
		int newY = y + i * dy_;
		if (newX < 0 || newX >= 14 || newY < 0 || newY >= 14)
			return false;
		if (m_dol[newY][newX] != m_dol_state_ + 1)
			return false;
		count += 1;
		if (count >= 5) {
			return true;
		}
	}
	return false;
}
bool CServerChat::CheckWin(int x, int y, int m_dol_state_) {
	if (CheckFive(x, y, 1, 0, m_dol_state_) //가로
		|| CheckFive(x, y, 0, 1, m_dol_state_) //세로
		|| CheckFive(x, y, 1, 1, m_dol_state_) //대각선 
		|| CheckFive(x, y, 1, -1, m_dol_state_))//대각선
		return true;
	return false;
}
//헤더로 뺼 것들 

void CServerChat::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (isServerTurn == FALSE) return;
	//WaitForSingleObject(hClientEvent, INFINITE);
	

	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CClientDC dc(this);
	
	int x = (point.x + 20) / 40;
	int y = (point.y + 20) / 40;

	if (m_dol[y - 1][x - 1] > 0) return;    //중복 체크 
	
	if (x > 0 && x <= 13 && y > 0 && y <= 13) {

		
		m_dol[y - 1][x - 1] = m_dol_state + 1;

		OnSendPosition(x, y);
		int dx_ = x;
		int dy_ = y;
		x *= 40;
		y *= 40;

		CBrush* p_old_brush;
		/*if (m_dol_state == 0)
		{
			p_old_brush = (CBrush*)dc.SelectStockObject(BLACK_BRUSH);
		}
		else
		{
			p_old_brush = (CBrush*)dc.SelectStockObject(WHITE_BRUSH);
		}*/

		//server는 흑돌
		p_old_brush = (CBrush*)dc.SelectStockObject(BLACK_BRUSH);

		dc.Ellipse(x - 20, y - 20, x + 20, y + 20);
		dc.SelectObject(p_old_brush);
		
		if (CheckWin(dx_ - 1, dy_ - 1, m_dol_state)) {
			// 게임 이겼을 경우
			//일단 이기면 좌표 창에 우승자 표시
			CString winString;
			winString = _T("Winner is ");
			m_orderList.AddString(winString + (!m_dol_state ? "black" : "white"));


			//게임 종료 화면 띄우기
			CGameover gameover;
			gameover.SetWinner(!m_dol_state ? 1 : 2); // 흑돌이 이겼으면 1, 백돌이 이겼으면 2
			gameover.DoModal();
		}


		
		CString str;
		str.Format(_T("x : %d y : %d"), x / 40, y / 40);
		

		m_orderList.AddString(str + " turn : " + "black");
		isServerTurn = FALSE;
	}
	
	CDialogEx::OnLButtonDown(nFlags, point);
}



//UINT CServerChat::ClientThread(LPVOID pParam)
//{
//	CServerChat* pThis = reinterpret_cast<CServerChat*>(pParam);
//	SOCKET clientSocket = (SOCKET)pParam;
//	CClientDC dc();
//	char buf[BUFSIZE];
//	int len;
//	pThis->ListInput();
//	while (true)
//	{
//		int retval = recv(clientSocket, (char*)&len, sizeof(int), 0);
//		if (retval <= 0) break;
//		if (len >= BUFSIZE) break;
//
//		retval = recv(clientSocket, buf, len, 0);
//		if (retval <= 0) break;
//		
//		buf[retval] = '\0';
//		
//		//채팅 메시지
//		// 동적 할당을 사용하지 않고 스택에 CString 객체를 생성
//		CString str(buf);
//		AfxMessageBox(_T("클라이언트로 부터 메시지 받기4"));
//		// 메시지를 UI 스레드로 전달
//		//pThis->PostMessage(M_SERVER_RECV_UPDATE, 0, (LPARAM)new CString(str));
//		pThis->ListInput();
//		AfxMessageBox(_T("클라이언트로 부터 메시지 받기5"));
//		
//		
//		
//		
//	}
//
//	// 소켓 닫기s
//	closesocket(clientSocket);
//	return 0;
//}

//void CServerChat::ListInput() {
//	AfxMessageBox(_T("여기 실행되나?"));
//	m_chatList.AddString(_T("please"));
//}