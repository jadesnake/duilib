#ifndef __UIHORIZONTALLAYOUT_H__
#define __UIHORIZONTALLAYOUT_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CHorizontalLayoutUI : public CContainerUI
	{
	public:
		CHorizontalLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetSepWidth(int iWidth);
		int GetSepWidth() const;
		void SetSepImmMode(bool bImmediately);
		bool IsSepImmMode() const;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoEvent(TEventUI& event);

		void SetPos(RECT rc);
		void DoPostPaint(HDC hDC, const RECT& rcPaint);

		RECT GetThumbRect(bool bUseNew = false) const;
	protected:
		bool  m_bAutoFixed;	//add by jade �Զ������߶�����
		int   m_iSepWidth;
		UINT  m_uButtonState;
		POINT ptLastMouse;
		RECT  m_rcNewPos;
		bool  m_bImmMode;
		UINT  m_unWay;		//fixed by jade 0-����룬1-�Ҷ���
	};
}
#endif // __UIHORIZONTALLAYOUT_H__
