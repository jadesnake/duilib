#include "stdafx.h"
#include "GridLayout.h"

namespace SvYui
{
	GridLayoutUI::GridLayoutUI() : m_unArrayNum(0)
	{
		m_mapAtbs	= new MAP_ATBS();
		m_vtLayoutH	= new VT_LAYOUT_H();
	}
	GridLayoutUI::GridLayoutUI(unsigned int arrayNum)
				 :m_unArrayNum(arrayNum)
	{
		m_mapAtbs	= new MAP_ATBS();
		m_vtLayoutH	= new VT_LAYOUT_H();
	}
	GridLayoutUI::~GridLayoutUI()
	{
		RemoveAll();

		delete m_mapAtbs;
		m_mapAtbs = NULL;

		delete m_vtLayoutH;
		m_vtLayoutH = NULL;
	}
	void GridLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		LPCTSTR pFlag = 0;
		if( 0 == _tcscmp(_T("arraynum"),pstrName) )
		{
			m_unArrayNum = _tstoi(pstrValue);
		}
		else if( pFlag = _tcsstr(pstrName,_T("_h")) )
		{
			 int nTotal = _tcslen(pstrName);
			 if( nTotal == (pFlag - pstrName +2) )
			 {
				 DuiLib::CDuiString strName=pstrName;
				 strName=strName.Left(nTotal-2);
				 (*m_mapAtbs)[strName] = pstrValue;
			 }
		}
		else
			DuiLib::CVerticalLayoutUI::SetAttribute(pstrName,pstrValue);
	}
	DuiLib::CHorizontalLayoutUI*	GridLayoutUI::appendArray(void)
	{
		DuiLib::CHorizontalLayoutUI *pLayout = new DuiLib::CHorizontalLayoutUI;
		if( pLayout )
		{
			if( DuiLib::CVerticalLayoutUI::Add(pLayout) )
			{
				MAP_ATBS::iterator	itPos = m_mapAtbs->begin();
				for(;itPos != m_mapAtbs->end();itPos++)
				{
					pLayout->SetAttribute(itPos->first,itPos->second);
				}

				m_vtLayoutH->push_back(pLayout);
			}
			else
			{
				delete pLayout;
				pLayout = 0;
			}
		}
		return pLayout;
	}
	bool GridLayoutUI::Add(DuiLib::CControlUI* pControl)
	{
		int nNum = GetCount();
		if( nNum == 0 )
		{
			DuiLib::CHorizontalLayoutUI *pArray = appendArray();
			if( pArray )
			{
				pArray->Add(pControl);
				return true;
			}
			return false;
		}
		DuiLib::CControlUI *pCtrl = __super::GetItemAt(nNum-1);
		if( (pCtrl != 0) && (0 == _tcscmp(_T("HorizontalLayoutUI"),pCtrl->GetClass())) )
		{
			DuiLib::CHorizontalLayoutUI *pLayout = static_cast<DuiLib::CHorizontalLayoutUI*>(pCtrl);
			if( m_unArrayNum <= pLayout->GetCount() )
			{
				DuiLib::CHorizontalLayoutUI *pArray = appendArray();
				if( pArray )
				{
					pArray->Add(pControl);
					return true;
				}
			}
			else
			{
				pLayout->Add( pControl );
				return true;
			}
		}
		return false;
	}
	bool GridLayoutUI::AddAt(DuiLib::CControlUI* pControl,int nX,int nY)
	{
		if( nY > m_unArrayNum )	return false;
		DuiLib::CControlUI *pCtrl = GetItemAt(nX);
		if( pCtrl == 0 )
		{
			DuiLib::CHorizontalLayoutUI *pArray = appendArray();
			if( pArray )
			{
				pArray->AddAt(pControl,nY);
				return true;
			}
		}
		else
		{
			DuiLib::CControlUI *pCtrl = GetItemAt(nX);
			if( (pCtrl != 0) && (0 == _tcscmp(_T("HorizontalLayoutUI"),pCtrl->GetClass())) )
			{
				DuiLib::CHorizontalLayoutUI *pLayout = static_cast<DuiLib::CHorizontalLayoutUI*>(pCtrl);
				if( m_unArrayNum <= pLayout->GetCount() )	return false;
				pLayout->AddAt(pControl,nY);
				return true;
			}
		}
		return false;
	}
	bool GridLayoutUI::Remove(DuiLib::CControlUI* pControl)
	{
		VT_LAYOUT_H::iterator itPos = m_vtLayoutH->begin();
		for(itPos;itPos != m_vtLayoutH->end();itPos++)
		{
			if( -1 != (*itPos)->GetItemIndex(pControl) )
				return (*itPos)->Remove(pControl);
		}
		return false;
	}
	bool GridLayoutUI::RemoveAt(int nX,int nY)
	{
		DuiLib::CControlUI *pCtrl = GetItemAt(nX);
		if( (pCtrl == 0) || (nY > m_unArrayNum) )	return false;
		if( (pCtrl != 0) && (0 == _tcscmp(_T("HorizontalLayoutUI"),pCtrl->GetClass())) )
		{
			DuiLib::CHorizontalLayoutUI *pLayout = static_cast<DuiLib::CHorizontalLayoutUI*>(pCtrl);
			return pLayout->RemoveAt(nY);
		}
		return false;
	}
	void GridLayoutUI::RemoveAll()
	{
		m_vtLayoutH->clear();
		__super::RemoveAll();
	}
};