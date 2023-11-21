#pragma once
#include "afxdialogex.h"


// CGameover 대화 상자

class CGameover : public CDialogEx
{
	DECLARE_DYNAMIC(CGameover)

public:
	CGameover(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CGameover();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GAMEOVER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	int m_nWinner;
	void SetWinner(int nWinner) { m_nWinner = nWinner; }
	CString m_strWinner;
};
