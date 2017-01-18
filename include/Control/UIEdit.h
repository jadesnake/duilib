#ifndef __UIEDIT_H__
#define __UIEDIT_H__

#pragma once

namespace DuiLib
{
	class CEditWnd;
	class CheckInput
	{
	public:
		virtual bool UserInput( TCHAR chChar ) = 0;
		virtual void Release() = 0;
	};
	class UILIB_API CEditUI : public CLabelUI
	{
		friend class CEditWnd;
	public:
		CEditUI();
		virtual ~CEditUI();
		void SetChecker( CheckInput *pChecker );
		CheckInput*	GetChecker();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(bool bEnable = true);
		void SetText(LPCTSTR pstrText);
		void SetMaxChar(UINT uMax);
		UINT GetMaxChar();
		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;
		void SetPasswordMode(bool bPasswordMode);
		bool IsPasswordMode() const;
		void SetPasswordChar(TCHAR cPasswordChar);
		TCHAR GetPasswordChar() const;
		void SetNumberOnly(bool bNumberOnly);
		bool IsNumberOnly() const;
		int GetWindowStyls() const;

		LPCTSTR GetNormalImage();
		void SetNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetHotImage();
		void SetHotImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusedImage();
		void SetFocusedImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage();
		void SetDisabledImage(LPCTSTR pStrImage);
		void SetNativeEditBkColor(DWORD dwBkColor);
		DWORD GetNativeEditBkColor() const;

		void SetSel(long nStartChar, long nEndChar);
		void SetSelAll();
		void SetReplaceSel(LPCTSTR lpszReplace);

		void SetPos(RECT rc);
		void SetVisible(bool bVisible = true);
		void SetInternVisible(bool bVisible = true);
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintStatusImage(HDC hDC);
		void PaintText(HDC hDC);

		void EnableFloat(bool bT);
		bool GetEnableFloat();
		
		void SetHotBorderColor(DWORD dwColor);
		virtual void PaintBorder(HDC hDC);

		//used for ServyouEvent
		virtual LRESULT OnEditChanged(LPTSTR text); //add by linwf at 2012-08-27
		virtual LRESULT OnEditKeyDown(WPARAM key);  //add by linwf at 2012-08-27
	protected:
		DWORD		m_dwHotBorderColor;		//add by jiayh
		CEditWnd	*m_pWindow;
		CheckInput	*m_pChecker;
		UINT m_uMaxChar;
		bool m_bReadOnly;
		bool m_bPasswordMode;
		TCHAR m_cPasswordChar;
		UINT m_uButtonState;
		CDuiString m_sNormalImage;
		CDuiString m_sHotImage;
		CDuiString m_sFocusedImage;
		CDuiString m_sDisabledImage;
		DWORD m_dwEditbkColor;
		int m_iWindowStyls;
		bool m_bEnableFloat;

	//add by changzz
	public:
		CEventSource OnEdit_Changed;
		CEventSource OnEdit_KeyDown;
	};
}
#endif // __UIEDIT_H__