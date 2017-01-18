#include "stdafx.h"
#include "UIButton.h"

namespace DuiLib
{
	CButtonUI::CButtonUI()
		: m_uButtonState(0)
		, m_dwHotTextColor(0)
		, m_dwPushedTextColor(0)
		, m_dwFocusedTextColor(0)
		, m_dwHotBkColor(0)
		, m_pUserCalFloatUI(0)
		, m_dwFloattipBkColor(0)
	{
		m_szPCT.cx = 3;
		m_szPCT.cy = 3;
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		memset(&m_szHot,0,sizeof(SIZE));
		memset(&m_rcHis,0,sizeof(RECT));
		memset(&m_rcRend,0,sizeof(RECT));
		m_pBtnPrev = new FloatBtns();
	}
	CButtonUI::~CButtonUI()
	{
		FloatBtns::iterator itPos = m_pBtnPrev->begin();
		for(itPos;itPos != m_pBtnPrev->end();itPos++)
			itPos->second->OnDestroy -= MakeDelegate(this,&DuiLib::CButtonUI::OnFireDestory);
		if( m_pUserCalFloatUI )
		{
			m_pUserCalFloatUI->OnRelease();
			m_pUserCalFloatUI = 0;
		}
		delete m_pBtnPrev;
		m_pBtnPrev = NULL;
	}
	CButtonUI*	CButtonUI::FindFloatBtn(SubBtnWay nWay)
	{
		FloatBtns::iterator itPos = m_pBtnPrev->find(nWay);
		if( itPos != m_pBtnPrev->end() )
			return itPos->second;
		return 0;
	}
	void CButtonUI::SetHotChange(const SIZE  &sz)
	{
		memcpy(&m_szHot,&sz,sizeof(SIZE));
	}
	const SIZE& CButtonUI::GetHotChange(void) const
	{
		return m_szHot;
	}
	void CButtonUI::SetFloatBtnPCT(UINT unH,UINT unW)
	{
		m_szPCT.cx = unH;
		m_szPCT.cy = unW;
	}
	const SIZE& CButtonUI::GetFloatBtnPCT(void)
	{
		return m_szPCT;
	}
	UINT CButtonUI::GetStatus(void)
	{
		return m_uButtonState;
	}
	const CButtonUI::CalFloatUI* CButtonUI::SetCalFloatUILogic( const CButtonUI::CalFloatUI *pUser )
	{
		CButtonUI::CalFloatUI* pRet = m_pUserCalFloatUI;
		m_pUserCalFloatUI = const_cast<CButtonUI::CalFloatUI*>(pUser);
		return pRet;
	}
	void CButtonUI::CalRect(CButtonUI *pUI,RECT rcItem,SubBtnWay nWay,CDuiRect &rcSub)
	{
		if( m_pManager == NULL ) return;
		if( rcItem.right < rcItem.left ) rcItem.right = rcItem.left;
		if( rcItem.bottom < rcItem.top ) rcItem.bottom = rcItem.top;
		CDuiRect duiRc(rcItem);
		CDuiRect duiTarget(rcItem);
		if( m_pUserCalFloatUI )
		{
			if( m_pUserCalFloatUI->Calculate(pUI,rcItem,nWay,rcSub) )
				return;
		}
		if( pUI->IsFloat() )
		{
			if( pUI->GetFixedXY().cx )
				duiTarget.left += pUI->GetFixedXY().cx;
			if( pUI->GetFixedXY().cy )
				duiTarget.top  += pUI->GetFixedXY().cy;

			if(  pUI->GetFixedWidth() )
				duiTarget.right = duiTarget.left+pUI->GetFixedWidth();
			else
				duiTarget.right = duiTarget.left+duiRc.GetWidth()/m_szPCT.cx;			
			if( pUI->GetFixedHeight() )
				duiTarget.bottom= duiTarget.top+pUI->GetFixedHeight();
			else
				duiTarget.bottom= duiTarget.top+duiRc.GetHeight()/m_szPCT.cy;
		}
		else
		{
			if( nWay == LeftTop )
			{
				if(  pUI->GetFixedWidth() )
					duiTarget.right = duiTarget.left+pUI->GetFixedWidth();			
				else
					duiTarget.right = duiTarget.left+duiRc.GetWidth()/m_szPCT.cx;			
				if( pUI->GetFixedHeight() )
					duiTarget.bottom= duiTarget.top+pUI->GetFixedHeight();
				else
					duiTarget.bottom= duiTarget.top+duiRc.GetHeight()/m_szPCT.cy;
			}
			else if( nWay == LeftBottom )
			{
				if( pUI->GetFixedWidth() )
					duiTarget.right = duiTarget.left+pUI->GetFixedWidth();
				else
					duiTarget.right = duiTarget.left+duiRc.GetWidth()/m_szPCT.cx;
				if( pUI->GetFixedHeight() )
					duiTarget.top	= duiRc.bottom-pUI->GetFixedHeight();
				else
					duiTarget.top	= duiRc.bottom-duiRc.GetHeight()/m_szPCT.cy;
			}
			else if( nWay == RightTop )
			{
				if( pUI->GetFixedWidth() )
					duiTarget.left  = duiTarget.right - pUI->GetFixedWidth();
				else
					duiTarget.left  = duiTarget.right - duiRc.GetWidth()/m_szPCT.cx;
				if( pUI->GetFixedHeight() )
					duiTarget.bottom= duiTarget.top + pUI->GetFixedHeight();
				else
					duiTarget.bottom= duiTarget.top + duiRc.GetHeight()/m_szPCT.cy;
			}
			else if( nWay == RightBottom )
			{
				if( pUI->GetFixedWidth() )
					duiTarget.left = duiTarget.right - pUI->GetFixedWidth();
				else
					duiTarget.left = duiTarget.right - duiRc.GetWidth()/m_szPCT.cx;
				if( pUI->GetFixedHeight() )
					duiTarget.top  = duiTarget.bottom- pUI->GetFixedHeight();
				else
					duiTarget.top  = duiTarget.bottom- duiRc.GetHeight()/m_szPCT.cy;
			}
		}
		rcSub = duiTarget;
	}
	void CButtonUI::CalFirstRect(FloatBtns *pBtns,CDuiRect &rcRet)
	{
		CDuiRect rcTmp(rcRet);
		FloatBtns::iterator itPos;
		CButtonUI *pLT=0,*pRT=0,*pLB=0,*pRB=0;
		bool bLT=false,bRT=false,bLB=false,bRB=false;
		pLT = FindFloatBtn(LeftTop);
		pRT = FindFloatBtn(RightTop);
		pLB = FindFloatBtn(LeftBottom);
		pRB = FindFloatBtn(RightBottom);
		if( pLT )
		{
			bLT = true;
		}
		if( pRT )
		{
			bRT = true;
		}
		if( pLB )
		{
			bLB = true;
		}
		if( pRB )
		{
			bRB = true;
		}
		if( bLT )
		{
			if( false == pLT->IsFloat() )
			{
				rcTmp.left += pLT->GetWidth()/2;
				rcTmp.top  += pLT->GetHeight()/2;
			}
		}
		if( bRT )
		{
			if( false == pRT->IsFloat() )
			{
				if( bLT==false  )
					rcTmp.top += pRT->GetHeight()/2;
				rcTmp.right -= pRT->GetWidth()/2;
			}
		}
		if( bLB )
		{
			if( false == pLB->IsFloat() )
			{
				if( bLT==false )
					rcTmp.left += pLB->GetWidth()/2;
				rcTmp.bottom -= pLB->GetHeight()/2;
			}
		}
		if( bRB )
		{
			if( false == pRB->IsFloat() )
			{
				if( bRT==false  )
					rcTmp.right  -= pRB->GetWidth()/2;
				if( bLB==false )
					rcTmp.bottom -= pRB->GetHeight()/2;
			}
		}
		rcRet = rcTmp;
	}
	bool CButtonUI::OnFireDestory(void *pObj)
	{
		//移除销毁的控件
		FloatBtns::iterator itPos = m_pBtnPrev->begin();
		for(itPos;itPos != m_pBtnPrev->end();itPos++)
		{
			if( itPos->second == pObj )
			{
				m_pBtnPrev->erase(itPos);
				break;
			}
		}
		return true;
	}
	LPCTSTR CButtonUI::GetClass() const
	{
		return _T("ButtonUI");
	}

	LPVOID CButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_BUTTON) == 0 ) return static_cast<CButtonUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CButtonUI::GetControlFlags() const
	{
		return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	void CButtonUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			OnOriginEvent(&event);
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			OnOriginEvent(&event);
			Invalidate();
		}
		if( event.Type == UIEVENT_KEYDOWN )
		{
			if (IsKeyboardEnabled())
			{
				if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) 
				{
					Activate();
					return;
				}
			}
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( m_pBtnPrev->size() )
			{
				CDuiRect rcTmp(m_rcItem),rcResult(m_rcItem);
				FloatBtns::iterator itPos = m_pBtnPrev->begin();
				for(itPos;itPos != m_pBtnPrev->end();itPos++)
				{
					if(  itPos->second->IsVisible() && itPos->second->IsEnabled() )
					{
						CalRect(itPos->second,rcTmp,itPos->first,rcResult);
						if( ::PtInRect(&rcResult,event.ptMouse) )
						{
							itPos->second->SetFocus();
							if (itPos->second->OnEvent((void*)&event))
							{
								itPos->second->DoEvent(event);
							}
							return ;
						}
					}					
				}
			}
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) 
			{
				SetFocus();
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				if (m_pBtnPrev->size())
				{
					OnOriginEvent(&event);
				}
				Invalidate();
			}
			return CLabelUI::DoEvent(event);
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( m_pBtnPrev->size() )
			{
				CDuiRect rcTmp(m_rcItem),rcResult(m_rcItem);
				FloatBtns::iterator itPos = m_pBtnPrev->begin();
				for(itPos;itPos != m_pBtnPrev->end();itPos++)
				{
					if(  itPos->second->IsVisible() && itPos->second->IsEnabled() )
					{
						CalRect(itPos->second,rcTmp,itPos->first,rcResult);
						if( ::PtInRect(&rcResult,event.ptMouse) )
						{
							if( IsEnabled() )	//取消当前按钮的hot状态
							{
								m_uButtonState &= ~UISTATE_HOT;
								Invalidate();
							}
							if( (itPos->second->GetStatus()&UISTATE_HOT) == 0 )
							{
								event.Type = UIEVENT_MOUSEENTER;
								itPos->second->DoEvent(event);
							}
							else
								itPos->second->DoEvent(event);
							return ;
						}
					}
				}
				CalFirstRect(m_pBtnPrev,rcTmp);
				if( ::PtInRect(&rcTmp, event.ptMouse) && IsEnabled() )
				{
					itPos = m_pBtnPrev->begin();
					for(itPos;itPos != m_pBtnPrev->end();itPos++)
					{
						CButtonUI *pCtrl = itPos->second;
						if( pCtrl->IsVisible() && pCtrl->IsEnabled() && ((UISTATE_HOT & pCtrl->GetStatus())==UISTATE_HOT) )
						{
							TEventUI eventUI;
							memcpy(&eventUI,&event,sizeof(TEventUI));
							eventUI.pSender = pCtrl;
							eventUI.Type    = UIEVENT_MOUSELEAVE;
							pCtrl->DoEvent(eventUI);
						}
					}
					m_uButtonState |= UISTATE_HOT;
					OnOriginEvent(&event);
					Invalidate();
				}
				else	//leave
				{
					if( IsEnabled() ) 
					{
						m_uButtonState &= ~UISTATE_HOT;
						OnOriginEvent(&event);
						Invalidate();
					}
				}
			}
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 )
			{				
				if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
				else m_uButtonState &= ~UISTATE_PUSHED;
				OnOriginEvent(&event);
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( m_pBtnPrev->size() )
			{
				CDuiRect rcTmp(m_rcItem),rcResult(m_rcItem);
				FloatBtns::iterator itPos = m_pBtnPrev->begin();
				for(itPos;itPos != m_pBtnPrev->end();itPos++)
				{
					if(  itPos->second->IsVisible() && itPos->second->IsEnabled() )
					{
						CalRect(itPos->second,rcTmp,itPos->first,rcResult);
						if( ::PtInRect(&rcResult,event.ptMouse) )
						{
							itPos->second->SetFocus();
							if (itPos->second->OnEvent((void*)&event))
							{
								itPos->second->DoEvent(event);
							}
							return ;
						}
					}
				}
			}
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 )
			{
				if( ::PtInRect(&m_rcItem, event.ptMouse) )
				{
					SetFocus();
					Activate();
				}
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				if (m_pBtnPrev->size())
				{
					OnOriginEvent(&event);
				}
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( IsContextMenuUsed() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( m_pBtnPrev->size() )
			{
				CDuiRect rcTmp(m_rcItem),rcResult(m_rcItem);
				FloatBtns::iterator itPos = m_pBtnPrev->begin();
				for(itPos;itPos != m_pBtnPrev->end();itPos++)
				{
					if(  itPos->second->IsVisible() && itPos->second->IsEnabled() )
					{
						CalRect(itPos->second,rcTmp,itPos->first,rcResult);
						if( ::PtInRect(&rcResult,event.ptMouse) )
						{
							itPos->second->DoEvent(event);
							return ;
						}
					}
				}
			}
			if( IsEnabled() ) 
			{
				m_uButtonState |= UISTATE_HOT;
				OnOriginEvent(&event);
				Invalidate();
			}
			// return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( m_pBtnPrev->size() )
			{
				CDuiRect rcTmp(m_rcItem);
				FloatBtns::iterator itPos = m_pBtnPrev->begin();
				for(itPos;itPos != m_pBtnPrev->end();itPos++)
				{
					CButtonUI *pCtrl = itPos->second;
					if( pCtrl->IsVisible() && pCtrl->IsEnabled() && ((UISTATE_HOT & pCtrl->GetStatus())==UISTATE_HOT) )
						itPos->second->DoEvent(event);
				}
			}
			if( IsEnabled() ) 
			{
				m_uButtonState &= ~UISTATE_HOT;
				OnOriginEvent(&event);
				Invalidate();
			}
			// return;
		}
		if( event.Type == UIEVENT_SETCURSOR ) {
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
			return;
		}
		CLabelUI::DoEvent(event);
	}

	bool CButtonUI::Activate()
	{
		if( !CControlUI::Activate() ) return false;
		if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
		return true;
	}

	void CButtonUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	//************************************
	// Method:    SetHotBkColor
	// FullName:  CButtonUI::SetHotBkColor
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: DWORD dwColor
	// Note:	  
	//************************************
	void CButtonUI::SetHotBkColor( DWORD dwColor )
	{
		m_dwHotBkColor = dwColor;
	}

	//************************************
	// Method:    GetHotBkColor
	// FullName:  CButtonUI::GetHotBkColor
	// Access:    public 
	// Returns:   DWORD
	// Qualifier: const
	// Note:	  
	//************************************
	DWORD CButtonUI::GetHotBkColor() const
	{
		return m_dwHotBkColor;
	}

	void CButtonUI::SetHotTextColor(DWORD dwColor)
	{
		m_dwHotTextColor = dwColor;
	}

	DWORD CButtonUI::GetHotTextColor() const
	{
		return m_dwHotTextColor;
	}

	void CButtonUI::SetPushedTextColor(DWORD dwColor)
	{
		m_dwPushedTextColor = dwColor;
	}

	DWORD CButtonUI::GetPushedTextColor() const
	{
		return m_dwPushedTextColor;
	}

	void CButtonUI::SetFocusedTextColor(DWORD dwColor)
	{
		m_dwFocusedTextColor = dwColor;
	}

	DWORD CButtonUI::GetFocusedTextColor() const
	{
		return m_dwFocusedTextColor;
	}

	LPCTSTR CButtonUI::GetNormalImage()
	{
		return m_sNormalImage;
	}

	void CButtonUI::SetNormalImage(LPCTSTR pStrImage)
	{
		m_sNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHotImage()
	{
		return m_sHotImage;
	}

	void CButtonUI::SetHotImage(LPCTSTR pStrImage)
	{
		m_sHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetPushedImage()
	{
		return m_sPushedImage;
	}

	void CButtonUI::SetPushedImage(LPCTSTR pStrImage)
	{
		m_sPushedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetFocusedImage()
	{
		return m_sFocusedImage;
	}

	void CButtonUI::SetFocusedImage(LPCTSTR pStrImage)
	{
		m_sFocusedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetDisabledImage()
	{
		return m_sDisabledImage;
	}

	void CButtonUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	//************************************
	// Method:    GetForeImage
	// FullName:  CButtonUI::GetForeImage
	// Access:    public 
	// Returns:   LPCTSTR
	// Qualifier:
	// Note:	  
	//************************************
	LPCTSTR CButtonUI::GetForeImage()
	{
		return m_sForeImage;
	}

	//************************************
	// Method:    SetForeImage
	// FullName:  CButtonUI::SetForeImage
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: LPCTSTR pStrImage
	// Note:	  
	//************************************
	void CButtonUI::SetForeImage( LPCTSTR pStrImage )
	{
		m_sForeImage = pStrImage;
		Invalidate();
	}

	//************************************
	// Method:    GetHotForeImage
	// FullName:  CButtonUI::GetHotForeImage
	// Access:    public 
	// Returns:   LPCTSTR
	// Qualifier:
	// Note:	  
	//************************************
	LPCTSTR CButtonUI::GetHotForeImage()
	{
		return m_sHotForeImage;
	}

	//************************************
	// Method:    SetHotForeImage
	// FullName:  CButtonUI::SetHotForeImage
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: LPCTSTR pStrImage
	// Note:	  
	//************************************
	void CButtonUI::SetHotForeImage( LPCTSTR pStrImage )
	{
		m_sHotForeImage = pStrImage;
		Invalidate();
	}

	SIZE CButtonUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 8);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("floattip")) == 0 )
		{
			m_sFloattip = pstrValue;
		}
		if( _tcscmp(pstrName, _T("floattipbkcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			m_dwFloattipBkColor = _tcstoul(pstrValue, &pstr, 16);
		}
		if( _tcscmp(pstrName, _T("floatPCT")) == 0 )
		{
			LPTSTR pstr = NULL;
			m_szPCT.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_szPCT.cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr);
		}
		if( _tcscmp(pstrName, _T("hotchange")) == 0 )
		{
			LPTSTR pstr = NULL;
			m_szHot.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			m_szHot.cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr);    
		}	
		else if( _tcscmp(pstrName, _T("floatBtn_LT")) == 0 )
		{				
			if( m_pBtnPrev->end() == m_pBtnPrev->find(LeftTop) )
			{
				(*m_pBtnPrev)[LeftTop] = new CButtonUI;
				(*m_pBtnPrev)[LeftTop]->SetManager(GetManager(),0,false);
				(*m_pBtnPrev)[LeftTop]->OnDestroy += MakeDelegate(this,&DuiLib::CButtonUI::OnFireDestory);
			}
			(*m_pBtnPrev)[LeftTop]->ApplyAttributeList(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("floatBtn_RT")) == 0 )
		{
			if( m_pBtnPrev->end() == m_pBtnPrev->find(RightTop) )
			{
				(*m_pBtnPrev)[RightTop] = new CButtonUI;
				(*m_pBtnPrev)[RightTop]->SetManager(GetManager(),0,false);
				(*m_pBtnPrev)[RightTop]->OnDestroy += MakeDelegate(this,&DuiLib::CButtonUI::OnFireDestory);
			}
			(*m_pBtnPrev)[RightTop]->ApplyAttributeList(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("floatBtn_LB")) == 0 )
		{
			if( m_pBtnPrev->end() == m_pBtnPrev->find(LeftBottom) )
			{
				(*m_pBtnPrev)[LeftBottom] = new CButtonUI;
				(*m_pBtnPrev)[LeftBottom]->SetManager(GetManager(),0,false);
				(*m_pBtnPrev)[LeftBottom]->OnDestroy += MakeDelegate(this,&DuiLib::CButtonUI::OnFireDestory);
			}
			(*m_pBtnPrev)[LeftBottom]->ApplyAttributeList(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("floatBtn_RB")) == 0 )
		{
			if( m_pBtnPrev->end() == m_pBtnPrev->find(RightBottom) )
			{
				(*m_pBtnPrev)[RightBottom] = new CButtonUI;
				(*m_pBtnPrev)[RightBottom]->SetManager(GetManager(),0,false);
				(*m_pBtnPrev)[RightBottom]->OnDestroy += MakeDelegate(this,&DuiLib::CButtonUI::OnFireDestory);
			}
			(*m_pBtnPrev)[RightBottom]->ApplyAttributeList(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
		else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotforeimage")) == 0 ) SetHotForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotbkcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("hottextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("pushedtextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetPushedTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("focusedtextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetFocusedTextColor(clrColor);
		}		
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}
	void CButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		//绘图顺序：底层控件->上层控件
		CDuiRect rcTmp(m_rcItem),rcResult(m_rcItem);
		if( (UISTATE_HOT!=(m_uButtonState&UISTATE_HOT)) && m_szHot.cx )
		{
			rcTmp.left  -= (m_szHot.cx*-1)/2;
			rcTmp.right += (m_szHot.cx*-1)/2;
		}
		if( (UISTATE_HOT!=(m_uButtonState&UISTATE_HOT)) && m_szHot.cy )
		{
			rcTmp.top    -= (m_szHot.cy*-1)/2;
			rcTmp.bottom += (m_szHot.cy*-1)/2;
		}
		if( m_pBtnPrev->size() && (FALSE == ::EqualRect(&m_rcHis,&m_rcItem)) )
		{
			memcpy(&m_rcHis,&m_rcItem,sizeof(RECT));

			FloatBtns::iterator itPos = m_pBtnPrev->begin();
			for(itPos;itPos != m_pBtnPrev->end();itPos++)
			{
				CalRect(itPos->second,rcTmp,itPos->first,rcResult);
				itPos->second->SetPos(rcResult);
			}
			CalFirstRect(m_pBtnPrev,rcTmp);

			::CopyRect(&m_rcRend,&rcTmp);			//储存按钮的绘制区域
		}
		else if( m_pBtnPrev->empty() )
		{
			if(FALSE == ::EqualRect(&m_rcRend,&m_rcItem)) 
				memcpy(&m_rcRend,&m_rcItem,sizeof(RECT));	//储存按钮的绘制区域
			if(FALSE == ::EqualRect(&m_rcHis,&m_rcItem))
				memcpy(&m_rcHis,&m_rcItem,sizeof(RECT));
		}
		if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) 
		{
			memcpy(&m_rcItem,&m_rcHis,sizeof(RECT));
			return;
		}
		// 绘制循序：背景颜色->背景图->状态图->文本->边框
		if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 )
		{
			CRenderClip roundClip;
			CRenderClip::GenerateRoundClip(hDC, m_rcPaint,  m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, roundClip);
			PaintBkColor(hDC);
			PaintBkImage(hDC);
			//采用计算后的效果绘制前景图
			memcpy(&m_rcItem,&m_rcRend,sizeof(RECT));
			PaintStatusImage(hDC);
			PaintText(hDC);
			PaintBorder(hDC);
		}
		else
		{
			PaintBkColor(hDC);
			PaintBkImage(hDC);
			//采用计算后的效果绘制前景图
			memcpy(&m_rcItem,&m_rcRend,sizeof(RECT));
			PaintStatusImage(hDC);
			PaintText(hDC);
			PaintBorder(hDC);
		}
		//绘制浮动按钮
		if( m_pBtnPrev->size() )
		{
			FloatBtns::iterator itPos = m_pBtnPrev->begin();
			for(itPos;itPos != m_pBtnPrev->end();itPos++)
			{
				if( itPos->second->IsVisible() )
					itPos->second->DoPaint(hDC,rcPaint);
			}
		}
		//恢复底层原始RECT
		memcpy(&m_rcItem,&m_rcHis,sizeof(RECT));
		//绘制浮动提示信息
		DoPainFloattip(hDC);
	}
	void CButtonUI::DoPainFloattip(HDC hDC)
	{
		CDuiRect rcItem(m_rcItem);
		rcItem.top += rcItem.GetHeight()/2;
		if( m_uButtonState&UISTATE_HOT )
		{
			if( m_dwFloattipBkColor )
				CRenderEngine::DrawGradient(hDC,rcItem,GetAdjustColor(m_dwFloattipBkColor),0,true,32);
			if( !m_sFloattip.IsEmpty() )
				CRenderEngine::DrawText(hDC,m_pManager,rcItem,m_sFloattip,m_dwTextColor,m_iFont,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		}
	}
	void CButtonUI::PaintText(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if( m_sText.IsEmpty() ) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
			clrColor = GetPushedTextColor();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
			clrColor = GetHotTextColor();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
			clrColor = GetFocusedTextColor();

		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, clrColor, \
			NULL, NULL, nLinks, m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, clrColor, \
			m_iFont, m_uTextStyle);
	}

	void CButtonUI::PaintStatusImage(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.IsEmpty() )
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
				else goto Label_ForeImage;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sPushedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) ){
					m_sPushedImage.Empty();
				}
				if( !m_sPushedForeImage.IsEmpty() )
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sPushedForeImage) )
						m_sPushedForeImage.Empty();
					return;
				}
				else goto Label_ForeImage;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHotImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ){
					m_sHotImage.Empty();
				}
				if( !m_sHotForeImage.IsEmpty() ) {
					if( !DrawImage(hDC, (LPCTSTR)m_sHotForeImage) )
						m_sHotForeImage.Empty();
					return;
				}
				else goto Label_ForeImage;
			}
			else if(m_dwHotBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHotBkColor));
				return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
				else goto Label_ForeImage;
			}
		}

		if( !m_sNormalImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
			else goto Label_ForeImage;
		}

		if(!m_sForeImage.IsEmpty() )
			goto Label_ForeImage;

		return;

Label_ForeImage:
		if(!m_sForeImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
		}
	}
}