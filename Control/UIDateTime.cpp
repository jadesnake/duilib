#include "stdafx.h"
#include "UIDateTime.h"

namespace DuiLib
{
	//CDateTimeUI::m_nDTUpdateFlag
#define DT_NONE   0
#define DT_UPDATE 1
#define DT_DELETE 2
#define DT_KEEP   3

	class CDateTimeWnd : public CWindowWnd
	{
	public:
		CDateTimeWnd();

		void Init(CDateTimeUI* pOwner);
		RECT CalPos();

		LPCTSTR GetWindowClassName() const;
		LPCTSTR GetSuperClassName() const;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	protected:
		CDateTimeUI* m_pOwner;
		HBRUSH m_hBkBrush;
		bool m_bInit;
		bool m_bUpDown;
	};

	CDateTimeWnd::CDateTimeWnd() : m_pOwner(NULL), m_hBkBrush(NULL), m_bInit(false)
	{
		m_bUpDown=false;
	}

	void CDateTimeWnd::Init(CDateTimeUI* pOwner)
	{
		m_pOwner = pOwner;
		m_pOwner->m_nDTUpdateFlag = DT_NONE;

		if (m_hWnd == NULL)
		{
			RECT rcPos = CalPos();
			UINT uStyle = WS_CHILD;
			Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
			SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);
		}

		if (m_pOwner->GetText().IsEmpty())
			::GetLocalTime(&m_pOwner->m_sysTime);

		::SendMessage(m_hWnd, DTM_SETSYSTEMTIME, 0, (LPARAM)&m_pOwner->m_sysTime);
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		::SetFocus(m_hWnd);

		m_bInit = true;    
	}

	RECT CDateTimeWnd::CalPos()
	{
		CDuiRect rcPos = m_pOwner->GetPos();
		return rcPos;
	}

	LPCTSTR CDateTimeWnd::GetWindowClassName() const
	{
		return _T("DateTimeWnd");
	}

	LPCTSTR CDateTimeWnd::GetSuperClassName() const
	{
		return DATETIMEPICK_CLASS;
	}

	void CDateTimeWnd::OnFinalMessage(HWND /*hWnd*/)
	{
		// Clear reference and die
		if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
		m_pOwner->m_pWindow = NULL;
		delete this;
	}

	LRESULT CDateTimeWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if( uMsg == WM_KILLFOCUS )
		{
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		}
		else if (uMsg == WM_KEYUP && (wParam == VK_DELETE || wParam == VK_BACK))
		{
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			//m_pOwner->m_nDTUpdateFlag = DT_DELETE;
			//m_pOwner->UpdateText();
			// by linwf
			PostMessage(WM_CLOSE);
			return lRes;
		}
		else if (uMsg == WM_KEYUP && wParam == VK_ESCAPE)
		{
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			m_pOwner->m_nDTUpdateFlag = DT_KEEP;
			PostMessage(WM_CLOSE);
			return lRes;
		}
		//add by linwf 2012-10-25
		else if (uMsg == OCM_NOTIFY)
		{
			LPNMHDR lpNMHDR = (LPNMHDR) lParam;
			if (lpNMHDR->code == DTN_DROPDOWN)
			{
				m_bUpDown = true;
			}
			else if (lpNMHDR->code == DTN_CLOSEUP)
			{
				::SetFocus(m_hWnd);
				m_bUpDown = false;
			}
			//------------------- Note by changzz -------------------
			//同时修改年或月或日时，第二次以后的修改就无效了
			//时间控件时间变化时，没有必要更新文本 - 更新 m_nDTUpdateFlag 的状态会影响 OnKillFocus
			//但XP系统的 OnKillFocus 是在用户通过日期选择窗口单击一下就触发的，和Win7系统不一样，所以还不可注释
			else if (lpNMHDR->code == DTN_DATETIMECHANGE)
			{
				::SendMessage(m_hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&m_pOwner->m_sysTime);
				m_pOwner->m_nDTUpdateFlag = DT_UPDATE;
				m_pOwner->UpdateText();
			}
			//------------------- /Note by changzz -------------------
			CDuiString	strTemp;
			strTemp.Format(_T("%d"),lpNMHDR->code);
			OutputDebugString(strTemp);
		}
		//	else if( uMsg == OCM_COMMAND ) {
		// 		if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
		// 		else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
		// 			RECT rcClient;
		// 			::GetClientRect(m_hWnd, &rcClient);
		// 			::InvalidateRect(m_hWnd, &rcClient, FALSE);
		// 		}
		//	}
		//	else if( uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_RETURN ) {
		// 		m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_RETURN);
		//	}
		// 		else if( uMsg == OCM__BASE + WM_CTLCOLOREDIT  || uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) {
		// 			if( m_pOwner->GetNativeEditBkColor() == 0xFFFFFFFF ) return NULL;
		// 			::SetBkMode((HDC)wParam, TRANSPARENT);
		// 			DWORD dwTextColor = m_pOwner->GetTextColor();
		// 			::SetTextColor((HDC)wParam, RGB(GetBValue(dwTextColor),GetGValue(dwTextColor),GetRValue(dwTextColor)));
		// 			if( m_hBkBrush == NULL ) {
		// 				DWORD clrColor = m_pOwner->GetNativeEditBkColor();
		// 				m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
		// 			}
		// 			return (LRESULT)m_hBkBrush;
		// 		}
		else bHandled = FALSE;
		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CDateTimeWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		//------------------- Modified by changzz -------------------
		//解决用户多次修改日期，最后一次修改未起作用的问题
		if (m_pOwner->m_nDTUpdateFlag == DT_NONE || m_pOwner->m_nDTUpdateFlag == DT_UPDATE)
		{
			::SendMessage(m_hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&m_pOwner->m_sysTime);
			m_pOwner->m_nDTUpdateFlag = DT_UPDATE;
			m_pOwner->UpdateText();
		}
		/*PostMessage(WM_CLOSE);*/
		
		if(!m_bUpDown)
			PostMessage(WM_CLOSE);
		return lRes;
	}

	// LRESULT CDateTimeWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	// {
	// 	if( !m_bInit ) return 0;
	// 	if( m_pOwner == NULL ) return 0;
	// 	// Copy text back
	// 	int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
	// 	LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
	// 	ASSERT(pstr);
	// 	if( pstr == NULL ) return 0;
	// 	::GetWindowText(m_hWnd, pstr, cchLen);
	// 	m_pOwner->m_sText = pstr;
	// 	m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_TEXTCHANGED);
	// 	return 0;
	// }

	//////////////////////////////////////////////////////////////////////////
	//
	CDateTimeUI::CDateTimeUI()
	{
		::GetLocalTime(&m_sysTime);
		m_sOldText.SmallFormat(_T("%4d-%02d-%02d"),
			m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay, m_sysTime.wHour, m_sysTime.wMinute);
		m_bReadOnly = false;
		m_pWindow = NULL;
		m_nDTUpdateFlag=DT_UPDATE;
		UpdateText();		// add by:daviyang35 初始化界面时显示时间
		m_nDTUpdateFlag = DT_NONE;
		
	}

	LPCTSTR CDateTimeUI::GetClass() const
	{
		return _T("DateTimeUI");
	}

	LPVOID CDateTimeUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_DATETIME) == 0 ) return static_cast<CDateTimeUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	SYSTEMTIME& CDateTimeUI::GetTime()
	{
		return m_sysTime;
	}

	void CDateTimeUI::SetTime(SYSTEMTIME* pst)
	{
		m_sysTime = *pst;
		Invalidate();
	}

	void CDateTimeUI::SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		Invalidate();
	}

	bool CDateTimeUI::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	LRESULT CDateTimeUI::OnEditChanged(LPTSTR text)
	{
		CDuiString cText = text;
		if (m_sOldText.Compare(cText))
		{
			m_sOldText = cText;
			m_pManager->SendNotify(this,DUI_MSGTYPE_TEXTCHANGED,0,(LPARAM)text);			
		}
		return 0;
	}

	void CDateTimeUI::UpdateText()
	{
		if (m_nDTUpdateFlag == DT_DELETE)
		{
			SetText(_T(""));
			OnEditChanged(_T(""));
		}
		else if (m_nDTUpdateFlag == DT_UPDATE)
		{
			CDuiString sText;
			sText.SmallFormat(_T("%4d-%02d-%02d"),
				m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay, m_sysTime.wHour, m_sysTime.wMinute);
			SetText(sText);
			OnEditChanged((LPTSTR)sText.GetData());
		}
	}

	void CDateTimeUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			return;
		}
		if( event.Type == UIEVENT_WINDOWSIZE )
		{
			if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if( m_pWindow != NULL ) return;
		}
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
		{
			if( m_pWindow ) return;
			m_pWindow = new CDateTimeWnd();
			ASSERT(m_pWindow);
			m_pWindow->Init(this);
			m_pWindow->ShowWindow();
		}
		if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
		{
			if( IsEnabled() ) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL )
				{
					m_pWindow = new CDateTimeWnd();
					ASSERT(m_pWindow);
				}
				if( m_pWindow != NULL )
				{
					m_pWindow->Init(this);
					m_pWindow->ShowWindow();
				}
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			return;
		}

		CLabelUI::DoEvent(event);
	}
}
