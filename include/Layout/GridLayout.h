#if !defined(__SERVYOU_UILIB_GRIDLAYOUT_H_)
#define __SERVYOU_UILIB_GRIDLAYOUT_H_
#include <map>
#include <vector>
namespace SvYui
{
	class UILIB_API GridLayoutUI : public DuiLib::CVerticalLayoutUI
	{
	public:
		typedef std::map<DuiLib::CDuiString,DuiLib::CDuiString> MAP_ATBS;
		typedef std::vector<DuiLib::CHorizontalLayoutUI*>	    VT_LAYOUT_H;
		GridLayoutUI();
		GridLayoutUI(unsigned int arrayNum);
		virtual ~GridLayoutUI();
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		bool Add(DuiLib::CControlUI* pControl);
		bool AddAt(DuiLib::CControlUI* pControl,int nX,int nY);
		bool Remove(DuiLib::CControlUI* pControl);
		bool RemoveAt(int nX,int nY);
		void RemoveAll();
		DuiLib::CHorizontalLayoutUI*	appendArray(void);
		LPCTSTR GetClass() const
		{	return _T("GridLayoutUI");	}
	private:
		int	          m_unArrayNum;	//列数
		MAP_ATBS*	  m_mapAtbs;	//属性列表，行
		VT_LAYOUT_H*  m_vtLayoutH;  //行
	};
};

#endif /*__SERVYOU_UILIB_GRIDLAYOUT_H_*/