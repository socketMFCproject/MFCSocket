// CClientChat.cpp: 구현 파일
//
#include <iostream>

#include "pch.h"
#include "SocketChat_improve.h"
#include "afxdialogex.h"
#include "CClientChat.h"
//#include "Common.h"
#include <thread>

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512
#define cell_size 40

#define M_RECV_UPDATE (WM_USER + 1) // 사용자 정의 메시지

// CClientChat 대화 상자

IMPLEMENT_DYNAMIC(CClientChat, CDialogEx)

CClientChat::CClientChat(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_clientMsg(_T(""))
{

}

//std::thread m_receiveThread;

CClientChat::~CClientChat()
{
	//if (m_receiveThread.joinable()) {
	//	m_receiveThread.join();
	//}
}

void CClientChat::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_clientMsg);
	//  DDX_Control(pDX, IDC_Client_LIST, m_chatList);
	//  DDX_Control(pDX, IDC_Client_LIST, m_chatList_client);
	DDX_Control(pDX, IDC_LIST2, m_orderList);
	DDX_Control(pDX, IDC_CLIENT_LIST_TEST, m_client_chat_list);
}


BEGIN_MESSAGE_MAP(CClientChat, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CLIENT_MSG_BUTTON, &CClientChat::OnBnClickedClientMsgButton)
	ON_BN_CLICKED(IDC_END_BUTTON, &CClientChat::OnBnClickedEndButton)
	ON_MESSAGE(M_RECV_UPDATE, OnUpdateListbox)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CClientChat::OnPaint()
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

// CClientChat 메시지 처리기

UINT CClientChat::ClientOwnThread(LPVOID aParam)
{
	CClientChat* pThis = reinterpret_cast<CClientChat*>(aParam);

	char buf[BUFSIZE];
	int len;

	while (true)
	{
		int retval = recv(pThis->m_sock, (char*)&len, sizeof(int), 0);
		if (retval <= 0) break;

		if (len >= BUFSIZE) break;

		retval = recv(pThis->m_sock, buf, len, 0);
		if (retval <= 0) break;

		buf[retval] = '\0';

		// PostMessage를 사용하여 메시지를 UI 스레드로 전달
		pThis->PostMessage(M_RECV_UPDATE, 0, (LPARAM)new CString(buf));

	}

	// 소켓 닫기
	closesocket(pThis->m_sock);
	pThis->m_sock = INVALID_SOCKET;
	return 0;
}


LRESULT CClientChat::OnUpdateListbox(WPARAM wParam, LPARAM lParam) {

	CString* pStr = reinterpret_cast<CString*>(lParam);
	m_client_chat_list.AddString(pStr->GetString());
	delete pStr; // 동적으로 할당된 CString 객체를 삭제
	return 0;
}

BOOL CClientChat::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0);

	// 소켓 생성
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	//if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) { AfxMessageBox(_T("connect 오류")); }
	AfxMessageBox(_T("서버 연결 성공"));
	//m_receiveThread = std::thread(&CClientChat::receiveMessages, this);
	//m_receiveThread.detach();
	AfxBeginThread(ClientOwnThread, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CClientChat::OnBnClickedClientMsgButton()
{

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	int len;

	// 서버와 데이터 통신
	CString msg = m_clientMsg;
	CStringA strAnsi(msg); // 유니코드 CString을 ANSI 문자열로 변환
	len = strAnsi.GetLength();

	// 버퍼 크기를 확인하고 널 종료 문자를 포함하여 복사
	strncpy_s(buf, BUFSIZE, strAnsi, len);

	// 데이터 보내기(고정 길이)
	int retval = send(m_sock, (char*)&len, sizeof(int), 0);
	// 데이터 보내기(가변 길이)
	retval = send(m_sock, buf, len, 0);

	m_client_chat_list.AddString(msg);

}


void CClientChat::OnBnClickedEndButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 소켓 닫기
	closesocket(m_sock);

	// 윈속 종료
	WSACleanup();
}


void CClientChat::OnLButtonDown(UINT nFlags, CPoint point)
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
