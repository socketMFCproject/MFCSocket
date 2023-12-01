// CGameover.cpp: 구현 파일
//

#include "pch.h"
#include "SocketChat_improve.h"
#include "afxdialogex.h"
#include "CGameover.h"


// CGameover 대화 상자

IMPLEMENT_DYNAMIC(CGameover, CDialogEx)

CGameover::CGameover(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GAMEOVER_DIALOG, pParent)
	, m_strWinner(_T(""))
{

}

CGameover::~CGameover()
{
}

void CGameover::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VICTORY, m_strWinner);
}


BEGIN_MESSAGE_MAP(CGameover, CDialogEx)
	ON_BN_CLICKED(IDC_GAMEOVER_BUTTON, &CGameover::OnBnClickedGameoverButton)
END_MESSAGE_MAP()


// CGameover 메시지 처리기


BOOL CGameover::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 글씨를 크게 하기 위한 폰트 객체 생성
	CFont m_font;
	m_font.CreatePointFont(160, _T("Arial")); // 16포인트 크기의 Arial 폰트

	// ID가 IDC_STATIC_WINNER인 컨트롤에 폰트 설정
	GetDlgItem(IDC_VICTORY)->SetFont(&m_font);

	if (m_nWinner == 1) {
		m_strWinner.SetString(_T("서버측이 승리했습니다."));
	}
	if (m_nWinner == 2) {
		m_strWinner.SetString(_T("클라이언트측이 승리했습니다."));
	}
	UpdateData(FALSE);

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CGameover::OnBnClickedGameoverButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	SendMessage(WM_CLOSE, 0, 0);
	CWnd* pMainWnd = AfxGetMainWnd(); // 메인 윈도우의 포인터를 가져옵니다.
	pMainWnd->SendMessage(WM_CLOSE); // 메인 윈도우에 종료 메시지를 보냅니다.

}
