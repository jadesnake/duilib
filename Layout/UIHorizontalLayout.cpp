#include "stdafx.h"
#include "UIHorizontalLayout.h"

namespace DuiLib
{
	CHorizontalLayoutUI::CHorizontalLayoutUI() 
		: m_iSepWidth(0), m_uButtonState(0),
		  m_bImmMode(false),m_unWay(0),m_bAutoFixed(false)
	{
		ptLastMouse.x = ptLastMouse.y = 0;
		::ZeroMemory(&m_rcNewPos, sizeof(m_rcNewPos));
	}

	LPCTSTR CHorizontalLayoutUI::GetClass() const
	{
		return _T("HorizontalLayoutUI");
	}

	LPVOID CHorizontalLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_HORIZONTALLAYOUT) == 0 ) return static_cast<CHorizontalLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CHorizontalLayoutUI::GetControlFlags() const
	{
		if( IsEnabled() && m_iSepWidth != 0 ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CHorizontalLayoutUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

		if( m_items.GetSize() == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the width of elements that are sizeable
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if( sz.cx == 0 ) {
				nAdjustables++;
			}
			else {
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx +  pControl->GetPadding().left + pControl->GetPadding().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * m_iChildPadding;

		int cxExpand = 0;
        int cxNeeded = 0;
		if( nAdjustables > 0 ) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int  iAdjustable = 0;
		int  cxFixedRemaining = cxFixed;
		if( m_unWay == 0 )
		{
			int iPosX = rc.left;
			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			{
				iPosX -= m_pHorizontalScrollBar->GetScrollPos();
			}
			for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) 
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
				if( !pControl->IsVisible() ) continue;
				if( pControl->IsFloat() ) 
				{
					SetFloatPos(it2);
					continue;
				}
				RECT rcPadding = pControl->GetPadding();
				szRemaining.cx -= rcPadding.left;
				SIZE sz = pControl->EstimateSize(szRemaining);
				if( sz.cx == 0 ) 
				{
					iAdjustable++;
					sz.cx = cxExpand;
					// Distribute remaining to last element (usually round-off left-overs)
					if( iAdjustable == nAdjustables ) 
					{
						sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
					}
					if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
					if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
				}
				else 
				{
					if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
					if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
					cxFixedRemaining -= sz.cx;
				}
				sz.cy = pControl->GetFixedHeight();
				if( sz.cy == 0 ) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
				if( sz.cy < 0 ) sz.cy = 0;
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
				RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy};
				pControl->SetPos(rcCtrl);
				iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
				cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
				szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
			}
		}
		else if( m_unWay == 1 )
		{
			int iPosX = rc.right;
			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
			{
				iPosX -= m_pHorizontalScrollBar->GetScrollPos();
			}
			for( int it2 = m_items.GetSize()-1; it2 >= 0 ; it2-- ) 
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
				if( !pControl->IsVisible() ) continue;
				if( pControl->IsFloat() ) 
				{
					SetFloatPos(it2);
					continue;
				}
				RECT rcPadding = pControl->GetPadding();
				szRemaining.cx -= rcPadding.left;
				SIZE sz = pControl->EstimateSize(szRemaining);
				if( sz.cx == 0 )
				{
					iAdjustable++;
					sz.cx = cxExpand;
					// Distribute remaining to last element (usually round-off left-overs)
					if( iAdjustable == nAdjustables )
					{
						sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
					}
					if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
					if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
				}
				else 
				{
					if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
					if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
					cxFixedRemaining -= sz.cx;
				}
				sz.cy = pControl->GetFixedHeight();
				if( sz.cy == 0 ) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
				if( sz.cy < 0 ) sz.cy = 0;
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
				RECT rcCtrl;
				if( iPosX == 0 )
				{
					rcCtrl.right = sz.cx + rcPadding.left + rcPadding.right;
					rcCtrl.left  = rcPadding.left;
					iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
				}
				else 
				{
					rcCtrl.right = iPosX - rcPadding.left - rcPadding.right;
					rcCtrl.left  = iPosX - sz.cx - rcPadding.left;
					iPosX = iPosX- sz.cx - m_iChildPadding - rcPadding.left - rcPadding.right;
				}
				rcCtrl.top	  = rc.top + rcPadding.top;
				rcCtrl.bottom = rc.top + rcPadding.top + sz.cy;
				pControl->SetPos(rcCtrl);
				szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
			}
		}
		cxNeeded += (nEstimateNum - 1) * m_iChildPadding;
		if( m_bAutoFixed && GetCount() )
		{
			CControlUI *pB = NULL;
			CControlUI *pE = NULL;
			int nBorder = GetLeftBorderSize()+GetRightBorderSize()+GetBorderSize();
			for( int nI=0;nI < GetCount();nI++ )
			{
				pB = GetItemAt(nI);
				if( pB->IsVisible() )
					break;
			}
			for( int nI=GetCount()-1;nI >= 0;nI-- )
			{
				pE = GetItemAt(nI);
				if( pE->IsVisible() )
					break;
			}
			if( pB && pE && (pB != pE) )
			{
				if( pB->IsVisible() && pE->IsVisible() )
				{
					RECT rcB = pB->GetPos();
					RECT rcE = pE->GetPos();
					long lW  = nBorder+rcE.right-rcB.left+pE->GetPadding().right+pB->GetPadding().left;
					if( GetFixedWidth() != lW )
						SetFixedWidth(lW);
				}
				else if( GetFixedWidth() )
				{
					m_rcItem.right = m_rcItem.left;
					SetFixedWidth(0);
				}
			}
			else if( pB )
			{
				if( pB->IsVisible() )
				{
					RECT rcB = pB->GetPos();
					long lW  = nBorder+rcB.right-rcB.left+pB->GetPadding().right+pB->GetPadding().left;
					if( GetFixedWidth() != lW )
						SetFixedWidth(lW);
				}
				else if( GetFixedWidth() )
				{
					m_rcItem.right = m_rcItem.left;
					SetFixedWidth(0);
				}
			}
			else if( pE )
			{
				if( pE->IsVisible() )
				{
					RECT rcE = pE->GetPos();
					long lW  = nBorder+rcE.right-rcE.left+pE->GetPadding().right+pE->GetPadding().left;
					if( GetFixedWidth() != lW )
						SetFixedWidth(lW);
				}
				else if( GetFixedWidth() )
				{
					m_rcItem.right = m_rcItem.left;
					SetFixedWidth(0);
				}
			}
		}
		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, 0);
	}

	void CHorizontalLayoutUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode ) {
			RECT rcSeparator = GetThumbRect(true);
			CRenderEngine::DrawColor(hDC, rcSeparator, 0xAA000000);
		}
	}

	void CHorizontalLayoutUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	int CHorizontalLayoutUI::GetSepWidth() const
	{
		return m_iSepWidth;
	}

	void CHorizontalLayoutUI::SetSepImmMode(bool bImmediately)
	{
		if( m_bImmMode == bImmediately ) return;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(this);
		}

		m_bImmMode = bImmediately;
	}

	bool CHorizontalLayoutUI::IsSepImmMode() const
	{
		return m_bImmMode;
	}

	void CHorizontalLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("way")) == 0 )
		{
			if( _tcscmp(pstrValue, _T("left")) == 0 )
				m_unWay = 0;
			else if( _tcscmp(pstrValue, _T("right")) == 0 )
				m_unWay = 1;
		}
		if( _tcscmp(pstrName, _T("autofixed")) == 0 )
		{
			if( _tcscmp(pstrValue, _T("true")) == 0 )
				m_bAutoFixed = true;
			else if( _tcscmp(pstrValue, _T("false")) == 0 )
				m_bAutoFixed = false;
		}
		else if( _tcscmp(pstrName, _T("sepwidth")) == 0 ) SetSepWidth(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("sepimm")) == 0 ) SetSepImmMode(_tcscmp(pstrValue, _T("true")) == 0);
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CHorizontalLayoutUI::DoEvent(TEventUI& event)
	{
		if( m_iSepWidth != 0 ) {
			if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
					m_rcNewPos = m_rcItem;
					if( !m_bImmMode && m_pManager ) m_pManager->AddPostPaint(this);
					return;
				}
			}
			if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
					m_rcItem = m_rcNewPos;
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cx = event.ptMouse.x - ptLastMouse.x;
					ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( m_iSepWidth >= 0 ) {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.right - m_iSepWidth ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.right ) return;
						rc.right += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.right = rc.left + GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.right = rc.left + GetMaxWidth();
						}
					}
					else {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.left ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.left - m_iSepWidth ) return;
						rc.left += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.left = rc.right - GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.left = rc.right - GetMaxWidth();
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;
					m_cxyFixed.cx = m_rcNewPos.right - m_rcNewPos.left;

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}
			if( event.Type == UIEVENT_SETCURSOR )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
					return;
				}
			}
		}
		CContainerUI::DoEvent(event);
	}

	RECT CHorizontalLayoutUI::GetThumbRect(bool bUseNew) const
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) {
			if( m_iSepWidth >= 0 ) return CDuiRect(m_rcNewPos.right - m_iSepWidth, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom);
			else return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - m_iSepWidth, m_rcNewPos.bottom);
		}
		else {
			if( m_iSepWidth >= 0 ) return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
			else return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
		}
	}
}
