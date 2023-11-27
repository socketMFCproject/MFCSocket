#pragma once
#include "afxdialogex.h"
//#include <thread>

// CClientChat 대화 상자

class CClientChat : public CDialogEx
{
	DECLARE_DYNAMIC(CClientChat)

public:
	CClientChat(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CClientChat();
private:
	int m_dol_state = 1; //0 : black(server), 1 : white(client)
	char m_dol[13][13];	//바둑돌 위치를 기억하기 위한 배열
	int myPortNum;
	int otherPortNum;
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	//std::thread m_receiveThread;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	SOCKET m_sock;
	virtual BOOL OnInitDialog();
	CString m_clientMsg;
	afx_msg void OnBnClickedClientMsgButton();
	afx_msg void OnBnClickedEndButton();
	afx_msg void OnPaint();
	afx_msg void OnSendPosition(int x, int y);
	afx_msg void SavePosition(int x, int y);
	afx_msg void OnSendMessage(char buf[], int len, int i);
	//헤더로 뺄 것들
	afx_msg bool CheckWin(int x, int y, int m_dol_state_);
	afx_msg bool CheckFive(int x, int y, int dx, int dy, int m_dol_state_);
	afx_msg void recivePoint(int x, int y, LPVOID aParam);
	afx_msg LRESULT OnUpdateListbox(WPARAM wParam, LPARAM lParam);
	//afx_msg static void receiveMessages(CClientChat* pDialog);
//	CListBox m_chatList;
//	CListBox m_chatList;
	CListBox m_orderList;

	static UINT ClientOwnThread(LPVOID aParam);
//	CListBox m_chatList_client;
	CListBox m_client_chat_list;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
