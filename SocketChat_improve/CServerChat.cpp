// CServerChat.cpp: 구현 파일
//

#include "pch.h"
#include "SocketChat_improve.h"
#include "afxdialogex.h"
#include "CServerChat.h"
//#include "Common.h"
#include <thread>

#define SERVERPORT 9000
#define BUFSIZE    512
#define cell_size 40

#define M_SERVER_RECV_UPDATE (WM_USER + 10) // 서버에서 사용할 사용자 정의 메시지
// CServerChat 대화 상자

IMPLEMENT_DYNAMIC(CServerChat, CDialogEx)

CServerChat::CServerChat(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_CHAT_DIALOG, pParent)
	, m_serverMsg(_T(""))
{

}

CServerChat::~CServerChat()
{
	closesocket(m_listenSocket);
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

		for (int y = 0; y < 12; y++)
		{
			for (int x = 0; x < 12; x++)
			{
				dc.Rectangle(cell_size + x * cell_size, cell_size + y * cell_size,
					cell_size + x * cell_size + cell_size + 1, cell_size + y * cell_size + cell_size + 1);
			}
		}

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

        std::lock_guard<std::mutex> lock(pThis->m_csClientSockets);
        pThis->m_clientSockets.push_back(clientSocket);
		
        //AfxBeginThread(ClientThread, (LPVOID)clientSocket);
		char buf[BUFSIZE];
		int len;
		while (true)
		{
			int retval = recv(clientSocket, (char*)&len, sizeof(int), 0);
			if (retval <= 0) break;
			if (len >= BUFSIZE) break;

			retval = recv(clientSocket, buf, len, 0);
			if (retval <= 0) break;

			buf[retval] = '\0';

			// 동적 할당을 사용하지 않고 스택에 CString 객체를 생성
			CString str(buf);
			
			// 메시지를 UI 스레드로 전달
			pThis->PostMessage(M_SERVER_RECV_UPDATE, 0, (LPARAM)new CString(str));
		
			
		}

		// 소켓 닫기s
		closesocket(clientSocket);

    }
    return 0;
}

void CServerChat::ListInput() {
	AfxMessageBox(_T("여기 실행되나?"));
	m_chatList.AddString(_T("please"));
}

UINT CServerChat::ClientThread(LPVOID pParam)
{
	CServerChat* pThis = reinterpret_cast<CServerChat*>(pParam);
	SOCKET clientSocket = (SOCKET)pParam;
	char buf[BUFSIZE];
	int len;
	pThis->ListInput();
	while (true)
	{
		int retval = recv(clientSocket, (char*)&len, sizeof(int), 0);
		if (retval <= 0) break;
		if (len >= BUFSIZE) break;

		retval = recv(clientSocket, buf, len, 0);
		if (retval <= 0) break;
		
		buf[retval] = '\0';
		
		// 동적 할당을 사용하지 않고 스택에 CString 객체를 생성
		CString str(buf);
		AfxMessageBox(_T("클라이언트로 부터 메시지 받기4"));
		// 메시지를 UI 스레드로 전달
		//pThis->PostMessage(M_SERVER_RECV_UPDATE, 0, (LPARAM)new CString(str));
		pThis->ListInput();
		AfxMessageBox(_T("클라이언트로 부터 메시지 받기5"));
	}

	// 소켓 닫기s
	closesocket(clientSocket);
	return 0;
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

	// TODO test해보자.


	//SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientaddr, &addrlen);

	AfxBeginThread(AcceptThread, this);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CServerChat::OnBnClickedServerSendButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	int len;

	// 서버와 데이터 통신
	CString msg = m_serverMsg;
	CStringA strAnsi(msg); // 유니코드 CString을 ANSI 문자열로 변환
	len = strAnsi.GetLength();

	// 버퍼 크기를 확인하고 널 종료 문자를 포함하여 복사
	strncpy_s(buf, BUFSIZE, strAnsi, len);

	SOCKET clientSocket = m_clientSockets[0];

	// 데이터 보내기(고정 길이)
	int retval = send(clientSocket, (char*)&len, sizeof(int), 0);
	// 데이터 보내기(가변 길이)
	retval = send(clientSocket, buf, len, 0);

	m_chatList.AddString(msg);

}


void CServerChat::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CClientDC dc(this);

	int x = (point.x + 20) / 40;
	int y = (point.y + 20) / 40;
	if (m_dol[y - 1][x - 1] > 0) return;    //중복 체크 
	m_dol[y - 1][x - 1] = m_dol_state + 1;        //바둑판 위치 저장
	if (x > 0 && x <= 13 && y > 0 && y <= 13) {
		x *= 40;
		y *= 40;

		CBrush* p_old_brush;
		if (m_dol_state == 0)
		{
			p_old_brush = (CBrush*)dc.SelectStockObject(BLACK_BRUSH);
		}
		else
		{
			p_old_brush = (CBrush*)dc.SelectStockObject(WHITE_BRUSH);
		}

		dc.Ellipse(x - 20, y - 20, x + 20, y + 20);
		dc.SelectObject(p_old_brush);
		m_dol_state = !m_dol_state;

		CString str_x, str_y;
		str_x.Format(_T("%d"), x / 40);
		str_y.Format(_T("%d"), y / 40);

		m_orderList.AddString(str_x + " y : " + str_y + " turn : " + (!m_dol_state ? "white" : "black"));

	}
	CDialogEx::OnLButtonDown(nFlags, point);
}
