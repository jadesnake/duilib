#pragma once

namespace DuiLib
{
	class UILIB_API CShadowWnd : public CWindowWnd, public INotifyUI
	{
	public:
		CShadowWnd(CPaintManagerUI* pOwnerPaintManager);
		virtual ~CShadowWnd();
		void UpdatePos(RECT rcShadowPadding);
		void MiniMize();
		void BeforeOwnerHandleMessage(UINT uMsg,WPARAM wParam,LPARAM lParam);
		void AfterOwnerHandleMessage(UINT uMsg,WPARAM wParam,LPARAM lParam);
		void AfterOwnerPaintManager(UINT uMsg,WPARAM wParam,LPARAM lParam);

	protected:
		virtual LPCTSTR GetWindowClassName() const { return _T("ShadowWnd"); }
		virtual void Notify(TNotifyUI& msg){};
		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		HRESULT OnCreate(WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		void ShowShadowWindow(bool bShow);

	private:
		CPaintManagerUI m_PaintManager;
		CPaintManagerUI* m_pOwnerPaintManager;
		CStdString m_strSkin;
		HWND m_hInnerWnd;
		bool m_bOwnerPainted;
		bool m_bShadowPainted;
		bool m_bShadowClosed;
	};
}