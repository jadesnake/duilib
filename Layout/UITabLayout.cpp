#include "stdafx.h"
#include "UITabLayout.h"

namespace DuiLib
{
	CTabLayoutUI::CTabLayoutUI() 
		: m_iCurSel(-1),m_bAutoFixedW(false),m_bAutoFixedH(false)
	{
	}

	LPCTSTR CTabLayoutUI::GetClass() const
	{
		return _T("TabLayoutUI");
	}

	LPVOID CTabLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_TABLAYOUT) == 0 ) return static_cast<CTabLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	bool CTabLayoutUI::Add(CControlUI* pControl)
	{
		bool ret = CContainerUI::Add(pControl);
		if( !ret ) return ret;

		if(m_iCurSel == -1 && pControl->IsVisible())
		{
			m_iCurSel = GetItemIndex(pControl);
		}
		else
		{
			pControl->SetVisible(false);
		}

		return ret;
	}

	bool CTabLayoutUI::AddAt(CControlUI* pControl, int iIndex)
	{
		bool ret = CContainerUI::AddAt(pControl, iIndex);
		if( !ret ) return ret;

		if(m_iCurSel == -1 && pControl->IsVisible())
		{
			m_iCurSel = GetItemIndex(pControl);
		}
		else if( m_iCurSel != -1 && iIndex <= m_iCurSel )
		{
			m_iCurSel += 1;
		}
		else
		{
			pControl->SetVisible(false);
		}

		return ret;
	}

	bool CTabLayoutUI::Remove(CControlUI* pControl)
	{
		if( pControl == NULL) return false;

		int index = GetItemIndex(pControl);
		bool ret = CContainerUI::Remove(pControl);
		if( !ret ) return false;

		if( m_iCurSel == index)
		{
			if( GetCount() > 0 )
			{
				m_iCurSel=0;
				GetItemAt(m_iCurSel)->SetVisible(true);
			}
			else
				m_iCurSel=-1;
			NeedParentUpdate();
		}
		else if( m_iCurSel > index )
		{
			m_iCurSel -= 1;
		}

		return ret;
	}

	void CTabLayoutUI::RemoveAll()
	{
		m_iCurSel = -1;
		CContainerUI::RemoveAll();
		NeedParentUpdate();
	}

	int CTabLayoutUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	bool CTabLayoutUI::SelectItem(int iIndex,bool bHasFocus)
	{
		if( iIndex < 0 || iIndex >= m_items.GetSize() ) return false;
		if( iIndex == m_iCurSel ) return true;

		int iOldSel = m_iCurSel;
		m_iCurSel = iIndex;
		for( int it = 0; it < m_items.GetSize(); it++ )
		{
			if( it == iIndex ) {
				GetItemAt(it)->SetVisible(true);
				if( bHasFocus )
					GetItemAt(it)->SetFocus();
				SetPos(m_rcItem);
			}
			else GetItemAt(it)->SetVisible(false);
		}
		NeedParentUpdate();

		if( m_pManager != NULL ) {
			if( bHasFocus )
				m_pManager->SetNextTabControl();
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABSELECT, m_iCurSel, iOldSel);
		}
		return true;
	}

	bool CTabLayoutUI::SelectItem( CControlUI* pControl,bool bHasFocus )
	{
		int iIndex=GetItemIndex(pControl);
		if (iIndex==-1)
			return false;
		else
			return SelectItem(iIndex,bHasFocus);
	}

	void CTabLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("autofixed_w")) == 0 )
		{
			if( _tcscmp(pstrValue, _T("true")) == 0 )
				m_bAutoFixedW = true;
			else if( _tcscmp(pstrValue, _T("false")) == 0 )
				m_bAutoFixedW = false;
		}
		if( _tcscmp(pstrName, _T("autofixed_h")) == 0 )
		{
			if( _tcscmp(pstrValue, _T("true")) == 0 )
				m_bAutoFixedH = true;
			else if( _tcscmp(pstrValue, _T("false")) == 0 )
				m_bAutoFixedH = false;
		}
		if( _tcscmp(pstrName, _T("selectedid")) == 0 ) 
			SelectItem(_ttoi(pstrValue));
		return CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CTabLayoutUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it);
				continue;
			}

			if( it != m_iCurSel ) continue;

			RECT rcPadding = pControl->GetPadding();
			rc.left += rcPadding.left;
			rc.top += rcPadding.top;
			rc.right -= rcPadding.right;
			rc.bottom -= rcPadding.bottom;

			SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

			SIZE sz = pControl->EstimateSize(szAvailable);
			if( sz.cx == 0 ) {
				sz.cx = MAX(0, szAvailable.cx);
			}
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

			if(sz.cy == 0) {
				sz.cy = MAX(0, szAvailable.cy);
			}
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();

			RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy};
			pControl->SetPos(rcCtrl);
		}
		if( m_bAutoFixedH && GetCount() )
		{
			int nBorder = GetTopBorderSize()+GetBottomBorderSize()+GetBorderSize();
			CControlUI* pSel = GetItemAt(GetCurSel());
			if( pSel )
			{
				if( pSel->IsVisible() )
				{
					long lH = pSel->GetPos().bottom - pSel->GetPos().top + \
							  pSel->GetPadding().top+pSel->GetPadding().bottom;
					if( lH != GetFixedHeight() )
						SetFixedHeight( lH );
				}
				else if( GetFixedHeight() )
				{
					SetFixedHeight( 0 );
				}
			}
		}
		if( m_bAutoFixedW && GetCount() )
		{
			int nBorder = GetTopBorderSize()+GetBottomBorderSize()+GetBorderSize();
			CControlUI* pSel = GetItemAt(GetCurSel());
			if( pSel )
			{
				if( pSel->IsVisible() )
				{
					long lW = pSel->GetPos().right - pSel->GetPos().left + \
							  pSel->GetPadding().left + pSel->GetPadding().right;
					if( lW != GetFixedWidth() )
						SetFixedWidth( lW );
				}
				else if( GetFixedWidth() )
				{
					SetFixedWidth(0);
				}
			}
		}
	}
}
