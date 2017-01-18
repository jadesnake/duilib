#include "stdafx.h"

namespace DuiLib
{

CShadowWnd::CShadowWnd(CPaintManagerUI* pOwnerPaintManager)
:m_bOwnerPainted(false),m_bShadowPainted(false),m_bShadowClosed(false)
{
	m_pOwnerPaintManager = pOwnerPaintManager;
	ASSERT(pOwnerPaintManager != NULL);
	m_strSkin = pOwnerPaintManager->GetShadowXml();
	m_hInnerWnd = pOwnerPaintManager->GetPaintWindow();
}

CShadowWnd::~CShadowWnd()
{

}

HRESULT CShadowWnd::OnCreate( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	m_PaintManager.Init(m_hWnd);
	CDialogBuilder builder;
	CControlUI* pRoot = builder.Create(m_strSkin.GetData(), (UINT)0, NULL, &m_PaintManager);
	m_PaintManager.AttachDialog(pRoot);
	m_PaintManager.AddNotifier(this);

	return 0;
}

LRESULT CShadowWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = TRUE;
	LRESULT lRes = 0;

	if (uMsg == WM_CREATE)
	{
		lRes = OnCreate(wParam,lParam,bHandled);
		if (bHandled)
		{
			return lRes;
		}
	}
	else if (uMsg == WM_NCCALCSIZE)
	{
		return lRes;
	}
	else if (uMsg == WM_SHOWWINDOW)
	{
		//增加此处判断是因为InnerWnd作为Owner，点击任务栏时，会将Shadow连带显示，导致如下现象：
		//步骤：1. 还原状态点任务栏最小化再还原；2. 最大化状态点任务栏最小化再还原。结果：阴影窗口未隐藏。
		if (wParam == TRUE)
		{
			if (IsMinimized(m_hInnerWnd) == TRUE || IsMaximized(m_hInnerWnd) == TRUE)
			{
				ShowShadowWindow(false);
				return lRes;
			}
		}
	}

	if( m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;

	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

void CShadowWnd::UpdatePos(RECT rcShadowPadding)
{
	if (!m_bOwnerPainted || m_bShadowClosed)
	{
		return;
	}

	if ( ::IsWindowVisible(m_hInnerWnd) == TRUE 
		&& IsMinimized(m_hInnerWnd) == FALSE && IsMaximized(m_hInnerWnd) == FALSE)
	{
		RECT rcPos = {0};
		::GetWindowRect(m_hInnerWnd,&rcPos);
		rcPos.left -= rcShadowPadding.left;
		rcPos.top -= rcShadowPadding.top;
		rcPos.right += rcShadowPadding.right;
		rcPos.bottom += rcShadowPadding.bottom;
		UINT uFlags = SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOCOPYBITS|SWP_SHOWWINDOW;
		::SetWindowPos(m_hWnd,m_hInnerWnd,rcPos.left,rcPos.top,rcPos.right-rcPos.left,
			rcPos.bottom-rcPos.top,uFlags);

		::InvalidateRect(m_hWnd,NULL,FALSE);
		::UpdateWindow(m_hWnd);
	}
	else
	{
		ShowShadowWindow(false);
	}
}

void CShadowWnd::MiniMize()
{
	PostMessage(WM_SYSCOMMAND,SC_MINIMIZE,0);
}

void CShadowWnd::BeforeOwnerHandleMessage( UINT uMsg,WPARAM wParam,LPARAM lParam )
{
	if (uMsg == WM_SHOWWINDOW)
	{
		if (wParam == TRUE)
		{
			UpdatePos(m_PaintManager.GetShadowPadding());
		}
		else
		{
			ShowShadowWindow(false);
			if (lParam == 0)
			{
				m_bOwnerPainted = false;
			}
		}
	}
}

void CShadowWnd::AfterOwnerHandleMessage( UINT uMsg,WPARAM wParam,LPARAM lParam )
{
	if (uMsg==WM_CLOSE||(uMsg==WM_SYSCOMMAND&&wParam==SC_CLOSE))
	{
		ShowShadowWindow(false);
		m_bShadowClosed = true;
		SendMessage(uMsg,wParam,lParam);
	}
	else if (uMsg == WM_WINDOWPOSCHANGED)
	{
		UpdatePos(m_PaintManager.GetShadowPadding());
	}
}

void CShadowWnd::AfterOwnerPaintManager(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_PAINT)
	{
		m_bOwnerPainted = true;
		if ( m_bShadowPainted == false )
		{
			UpdatePos(m_PaintManager.GetShadowPadding());
			m_bShadowPainted = true;
		}
		else
		{
			if (::IsWindowVisible(m_hWnd) == FALSE)
			{
				UpdatePos(m_PaintManager.GetShadowPadding());
			}
		}
	}
}

void CShadowWnd::ShowShadowWindow( bool bShow )
{
	if (m_bShadowClosed)
	{
		return;
	}
	if (bShow)
	{
		::SetWindowPos(m_hWnd,m_hInnerWnd,-1,-1,-1,-1,
			SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
	}
	else
	{
		::SetWindowPos(m_hWnd,m_hInnerWnd,-1,-1,-1,-1,
			SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_HIDEWINDOW);
	}
}

}