#pragma once
#include "afxdialogex.h"
//#include <iostream>
#include <vector>
#include <mutex>

// CServerChat 대화 상자

class CServerChat : public CDialogEx
{
	DECLARE_DYNAMIC(CServerChat)

public:
	CServerChat(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CServerChat();


private:
	int m_dol_state = 0; //0 : black(server), 1 : white(client)
	char m_dol[13][13];	//바둑돌 위치를 기억하기 위한 배열
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_CHAT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListBox m_chatList;
	CListBox m_orderList;
	CString m_serverMsg;
	CButton m_sendButton;
	afx_msg void OnBnClickedServerSendButton();
	afx_msg void OnSendPosition(int x, int y);
	afx_msg void SavePosition(int x, int y);
	//헤더로 뺄 것들
	afx_msg bool CheckFive(int x, int y, int dx, int dy);
	afx_msg bool CheckWin(int x, int y);
	afx_msg void recivePoint(int x, int y, LPVOID pParam);
	//헤더로 뺄 것들
	afx_msg void OnPaint();
	afx_msg LRESULT OnUpdateServerChat(WPARAM wParam, LPARAM lParam);
	
	SOCKET m_listenSocket;  // 리스닝 소켓 멤버
	
	std::vector<SOCKET> m_clientSockets; // 클라이언트 소켓을 관리할 벡터
	std::mutex m_csClientSockets; // 클라이언트 소켓 벡터를 보호할 뮤텍스

	static UINT AcceptThread(LPVOID pParam); // 클라이언트 연결 수락 스레드
	//static UINT ClientThread(LPVOID pParam); // 클라이언트 통신 스레드

	//afx_msg void ListInput(CString str);
	//afx_msg void ListInput();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
