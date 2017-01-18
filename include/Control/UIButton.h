#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

#pragma once
#include <map>

namespace DuiLib
{
	class UILIB_API CButtonUI : public CLabelUI
	{
	public:
		typedef enum
		{
			LeftTop,			//左上
			RightTop,			//右上
			LeftBottom,			//左下
			RightBottom			//右下
		}SubBtnWay;				//子按钮位置
		class CalFloatUI		//业务层自定义计算规则
		{
		public:
			virtual bool Calculate(CButtonUI *pUI,RECT rcItem,SubBtnWay nWay,CDuiRect &rcSub) = 0;
			virtual void OnRelease(void) = 0;
		};
		typedef std::map<SubBtnWay,CButtonUI*>	FloatBtns;

		CButtonUI();
		virtual ~CButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		bool Activate();
		void SetEnabled(bool bEnable = true);
		void DoEvent(TEventUI& event);

		LPCTSTR GetNormalImage();
		virtual void SetNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetHotImage();
		virtual void SetHotImage(LPCTSTR pStrImage);
		LPCTSTR GetPushedImage();
		virtual void SetPushedImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusedImage();
	 	virtual void SetFocusedImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage();
		virtual void SetDisabledImage(LPCTSTR pStrImage);
		LPCTSTR GetForeImage();
		virtual void SetForeImage(LPCTSTR pStrImage);
		LPCTSTR GetHotForeImage();
		virtual void SetHotForeImage(LPCTSTR pStrImage);

		void SetHotBkColor(DWORD dwColor);
		DWORD GetHotBkColor() const;
		void SetHotTextColor(DWORD dwColor);
		DWORD GetHotTextColor() const;
		void SetPushedTextColor(DWORD dwColor);
		DWORD GetPushedTextColor() const;
		void SetFocusedTextColor(DWORD dwColor);
		DWORD GetFocusedTextColor() const;
		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetFloatBtnPCT(UINT unH,UINT unW);
		const SIZE& GetFloatBtnPCT(void);
		CButtonUI*	FindFloatBtn(SubBtnWay nWay);
	 	const CalFloatUI*	SetCalFloatUILogic( const CalFloatUI *pUser );		
		void PaintText(HDC hDC);
		void PaintStatusImage(HDC hDC);
		void DoPaint(HDC hDC, const RECT& rcPaint);
		void DoPainFloattip(HDC hDC);

		UINT GetStatus(void);
		
		void SetHotChange(const SIZE  &sz);
		const SIZE& GetHotChange(void) const;
	public:
		CEventSource OnOriginEvent;
	protected:
		bool OnFireDestory(void *pObj);
		//通过当前对象RECT计算,图层二的RECT
		void CalRect(CButtonUI *pUI,RECT rcItem,SubBtnWay nWay,CDuiRect &rcSub);
		//通过当前对象RECT计算,图层一的RECT
		void CalFirstRect(FloatBtns *pBtns,CDuiRect &rcRet);
	protected:
		RECT		m_rcHis;			//历史控件RECT状态	
		
		RECT		m_rcRend;			//控件绘制的RECT

		SIZE		m_szHot;			//hotchange尺寸		
		SIZE		m_szPCT;			//宽度,高度比例尺
		FloatBtns	*m_pBtnPrev;		//上层操作按钮
		CalFloatUI	*m_pUserCalFloatUI;	//用户自定义计算规则式
		UINT m_uButtonState;

		DWORD		m_dwFloattipBkColor;		//浮动背景色
		CDuiString	m_sFloattip;				//浮动显示文字
		
		DWORD m_dwHotBkColor;
		DWORD m_dwHotTextColor;
		DWORD m_dwPushedTextColor;
		DWORD m_dwFocusedTextColor;

		CDuiString m_sNormalImage;
		CDuiString m_sHotImage;
		CDuiString m_sHotForeImage;
		CDuiString m_sPushedImage;
		CDuiString m_sPushedForeImage;
		CDuiString m_sFocusedImage;
		CDuiString m_sDisabledImage;
	};

}	// namespace DuiLib

#endif // __UIBUTTON_H__