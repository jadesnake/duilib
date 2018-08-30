#include "stdafx.h"

#pragma comment(lib,"Riched20.lib")

// These constants are for backward compatibility. They are the 
// sizes used for initialization and reset in RichEdit 1.0
const LONG cInitTextMax = (64 * 1024) - 1;

EXTERN_C const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
    0x8d33f740,
    0xcf58,
    0x11ce,
    {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
};

EXTERN_C const IID IID_ITextHost = { /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
    0xc5bdd8d0,
    0xd26e,
    0x11ce,
    {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
};
EXTERN_C const IID IID_ITextDocument = {  //8CC497C0-A1DF-11ce-8098-00AA0047BE5D
	0x8CC497C0, 
	0xA1DF, 
	0x11CE,
	{ 0x80, 0x98, 0x00, 0xaa, 0x00, 0x47, 0xbe, 0x5d } 
};
EXTERN_C const IID IID_ITextDocument2 = { //C241F5E0-7206-11D8-A2C7-00A0D1D6C6B3
	0xC241F5E0, 
	0x7206, 
	0x11D8,
	{ 0xA2, 0xC7, 0x00, 0xA0, 0xD1, 0xD6, 0xC6, 0xB3 } 
};

#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

#ifndef HIMETRIC_PER_INCH
#define HIMETRIC_PER_INCH 2540
#endif

#include <atlbase.h>
#include <textserv.h>
#include <Richole.h>
#include "tom2.h"
typedef HRESULT (STDAPICALLTYPE* DbgCreateTextServices)(IUnknown *punkOuter,ITextHost *pITextHost,IUnknown **ppUnk);
HMODULE LoadRichEdit(IUnknown *punkOuter,ITextHost *pITextHost,IUnknown **ppUnk)
{
	HMODULE hModule = LoadLibrary(_T("RICHED20.DLL"));
	if( hModule == NULL )
		return NULL;
	DbgCreateTextServices proc = (DbgCreateTextServices)GetProcAddress(hModule,"CreateTextServices");
	proc(punkOuter,pITextHost,ppUnk);
	return hModule;
}

void FireOleScroll(IRichEditOle* pRe,long nW,long nH)
{
	if( pRe == 0 )		return ;
	long  lCount = pRe->GetObjectCount();
	HRESULT hr = 0;
	for(int n=0;n < lCount;n++)
	{
		REOBJECT tgObj;
		tgObj.cbStruct = sizeof(REOBJECT);
		hr = pRe->GetObject(n,&tgObj,REO_GETOBJ_ALL_INTERFACES);
		if( hr != S_OK )	continue;
		if( tgObj.poleobj )
		{
			IOleInPlaceSite *pSite = NULL;
			tgObj.poleobj->QueryInterface(IID_IOleInPlaceSite,(void**)&pSite);
			if( pSite )
			{
				SIZE szScroll={nW,nH};
				pSite->Scroll(szScroll);
				pSite->Release();
			}
		}
		if( tgObj.poleobj )
			tgObj.poleobj->Release();
		if( tgObj.polesite )
			tgObj.polesite->Release();
		if( tgObj.pstg )
			tgObj.pstg->Release();
	}
}


namespace DuiLib {

UILIB_API LONG CalLogFontInchH(HDC dc,long h)
{
	LONG yPixPerInch = GetDeviceCaps(dc, LOGPIXELSY);
	return (h * LY_PER_INCH / yPixPerInch);
}

class CTxtWinHost : public ITextHost
{
public:
	enum UiState
	{
		ResizedRc  = 1,
		ReDrawItem,
		UpdateItems,
		ResetScoll,
		ResetCmp,
	};
	enum SysCmd			//用于记录系统事件
	{		
		SysMin	   = 1,	//系统最小化
		SysRestore	  , //系统还原
		SysEmpty		//空值
	};
    CTxtWinHost();
    BOOL Init(CRichEditUI *re , const CREATESTRUCT *pcs);
    virtual ~CTxtWinHost();

    ITextServices* GetTextServices(void) { return pserv; }
	ITextDocument2* GetTextDocument(void) { return pdoc;  }
    void SetClientRect(RECT *prc);
	void UpdateInset();

    RECT* GetClientRect() { return &rcClient; }
    BOOL GetWordWrap(void) { return fWordWrap; }
    void SetWordWrap(BOOL fWordWrap);
    BOOL GetReadOnly();
    void SetReadOnly(BOOL fReadOnly);
    void SetFont(HFONT hFont);
    void SetColor(DWORD dwColor);
    SIZEL* GetExtent();
    void SetExtent(SIZEL *psizelExtent);
    void LimitText(LONG nChars);
    BOOL IsCaptured();
	void NotifyDrawItem();

    BOOL GetAllowBeep();
    void SetAllowBeep(BOOL fAllowBeep);
    WORD GetDefaultAlign();
    void SetDefaultAlign(WORD wNewAlign);
    BOOL GetRichTextFlag();
    void SetRichTextFlag(BOOL fNew);
    LONG GetDefaultLeftIndent();
    void SetDefaultLeftIndent(LONG lNewIndent);
    BOOL SetSaveSelection(BOOL fSaveSelection);
    HRESULT OnTxInPlaceDeactivate();
    HRESULT OnTxInPlaceActivate(LPCRECT prcClient);
    BOOL GetActiveState(void) { return fInplaceActive; }
    BOOL DoSetCursor(RECT *prc, POINT *pt);
    void SetTransparent(BOOL fTransparent);
    void GetControlRect(LPRECT prc);
    LONG SetAccelPos(LONG laccelpos);
    WCHAR SetPasswordChar(WCHAR chPasswordChar);
    void SetDisabled(BOOL fOn);
    LONG SetSelBarWidth(LONG lSelBarWidth);
	void EnableScroll(DWORD dwScroll,BOOL bEnable);
    BOOL GetTimerState();

    void SetCharFormat(CHARFORMAT2W &c);
    void SetParaFormat(PARAFORMAT2 &p);

    // -----------------------------
    //	IUnknown interface
    // -----------------------------
    virtual HRESULT _stdcall QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG _stdcall AddRef(void);
    virtual ULONG _stdcall Release(void);

    // -----------------------------
    //	ITextHost interface
    // -----------------------------
    virtual HDC TxGetDC();
    virtual INT TxReleaseDC(HDC hdc);
    virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
    virtual BOOL TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);
    virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
    virtual BOOL TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);
    virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode);
    virtual void TxViewChange(BOOL fUpdate);
    virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
    virtual BOOL TxShowCaret(BOOL fShow);
    virtual BOOL TxSetCaretPos(INT x, INT y);
    virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
    virtual void TxKillTimer(UINT idTimer);
    virtual void TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
    virtual void TxSetCapture(BOOL fCapture);
    virtual void TxSetFocus();
    virtual void TxSetCursor(HCURSOR hcur, BOOL fText);
    virtual BOOL TxScreenToClient (LPPOINT lppt);
    virtual BOOL TxClientToScreen (LPPOINT lppt);
    virtual HRESULT TxActivate( LONG * plOldState );
    virtual HRESULT TxDeactivate( LONG lNewState );
    virtual HRESULT TxGetClientRect(LPRECT prc);
    virtual HRESULT TxGetViewInset(LPRECT prc);
    virtual HRESULT TxGetCharFormat(const CHARFORMATW **ppCF );
    virtual HRESULT TxGetParaFormat(const PARAFORMAT **ppPF);
    virtual COLORREF TxGetSysColor(int nIndex);
    virtual HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle);
    virtual HRESULT TxGetMaxLength(DWORD *plength);
    virtual HRESULT TxGetScrollBars(DWORD *pdwScrollBar);
    virtual HRESULT TxGetPasswordChar(TCHAR *pch);
    virtual HRESULT TxGetAcceleratorPos(LONG *pcp);
    virtual HRESULT TxGetExtent(LPSIZEL lpExtent);
    virtual HRESULT OnTxCharFormatChange (const CHARFORMATW * pcf);
    virtual HRESULT OnTxParaFormatChange (const PARAFORMAT * ppf);
    virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);
    virtual HRESULT TxNotify(DWORD iNotify, void *pv);
    virtual HIMC TxImmGetContext(void);
    virtual void TxImmReleaseContext(HIMC himc);
    virtual HRESULT TxGetSelectionBarWidth (LONG *lSelBarWidth);

	bool	 bThrowEvent;
	SIZE	 szScrollMove;
	float	 m_fScrollThum;			//
	DWORD	 m_ScrollStyle;
	UiState  _UiState;
	SysCmd	 _SysCmd;

	INT			m_CaretxWidth;			
	INT			m_CaretyHeight;
private:
    CRichEditUI *m_re;
    ULONG	cRefs;					// Reference Count
    ITextServices	*pserv;		    // pointer to Text Services object
	ITextDocument2	*pdoc;			//
    // Properties

    DWORD		dwStyle;				// style bits

    unsigned	fEnableAutoWordSel	:1;	// enable Word style auto word selection?
    unsigned	fWordWrap			:1;	// Whether control should word wrap
    unsigned	fAllowBeep			:1;	// Whether beep is allowed
    unsigned	fRich				:1;	// Whether control is rich text
    unsigned	fSaveSelection		:1;	// Whether to save the selection when inactive
    unsigned	fInplaceActive		:1; // Whether control is inplace active
    unsigned	fTransparent		:1; // Whether control is transparent
    unsigned	fTimer				:1;	// A timer is set
    unsigned    fCaptured           :1;

    LONG		lSelBarWidth;			// Width of the selection bar
    LONG  		cchTextMost;			// maximum text size
    DWORD		dwEventMask;			// DoEvent mask to pass on to parent window
    LONG		icf;
    LONG		ipf;
    RECT		rcClient;				// Client Rect for this control
    SIZEL		sizelExtent;			// Extent array
    CHARFORMAT2W cf;					// Default character format
    PARAFORMAT2	pf;					    // Default paragraph format
    LONG		laccelpos;				// Accelerator position
    WCHAR		chPasswordChar;		    // Password character

	RECT		m_rcInset;
	HMODULE		m_richedit;
};

// Convert Device Pixels to Himetric
LONG DtoHimetric(LONG d, LONG dPerInch)
{
	return (LONG) MulDiv(d, HIMETRIC_PER_INCH, dPerInch);
}
// Convert Himetric Device pixels
LONG HimetrictoD(LONG lHimetric, LONG dPerInch)
{
	return (LONG) MulDiv(lHimetric, dPerInch, HIMETRIC_PER_INCH);
}

HRESULT InitDefaultCharFormat(CRichEditUI* re, CHARFORMAT2W* pcf, HFONT hfont) 
{
    memset(pcf, 0, sizeof(CHARFORMAT2W));
    LOGFONT lf;
    if( !hfont )
        hfont = re->GetManager()->GetFont(re->GetFont());
    ::GetObject(hfont, sizeof(LOGFONT), &lf);

    DWORD dwColor = re->GetTextColor();
    pcf->cbSize = sizeof(CHARFORMAT2W);
    pcf->crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
    LONG yPixPerInch = GetDeviceCaps(re->GetManager()->GetPaintDC(), LOGPIXELSY);
    pcf->yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
    pcf->yOffset = 0;
    pcf->dwEffects = 0;
    pcf->dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
	if(lf.lfWeight >= FW_BOLD)
		pcf->dwEffects |= CFE_BOLD;
	else
		pcf->dwEffects &= ~CFE_BOLD;
	if(lf.lfItalic)
		pcf->dwEffects |= CFE_ITALIC;
	else
		pcf->dwEffects &= ~CFE_ITALIC;

	if(lf.lfUnderline)
		pcf->dwEffects |= CFE_UNDERLINE;
	else
		pcf->dwEffects &= ~CFE_UNDERLINE;
    pcf->bCharSet = lf.lfCharSet;
    pcf->bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
    _tcscpy(pcf->szFaceName, lf.lfFaceName);
#else
    //need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
    MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, pcf->szFaceName, LF_FACESIZE) ;
#endif

    return S_OK;
}

HRESULT InitDefaultParaFormat(CRichEditUI* re, PARAFORMAT2* ppf) 
{	
    memset(ppf, 0, sizeof(PARAFORMAT2));
    ppf->cbSize = sizeof(PARAFORMAT2);
    ppf->dwMask = PFM_ALL;
    ppf->wAlignment = PFA_LEFT;
    ppf->cTabCount = 1;
    ppf->rgxTabs[0] = lDefaultTab;

    return S_OK;
}

HRESULT CreateHost(CRichEditUI *re, const CREATESTRUCT *pcs, CTxtWinHost **pptec)
{
    HRESULT hr = E_FAIL;
    //GdiSetBatchLimit(1);

    CTxtWinHost *phost = new CTxtWinHost();
    if(phost)
    {
        if (phost->Init(re, pcs))
        {
            *pptec = phost;
            hr = S_OK;
        }
    }

    if (FAILED(hr))
    {
        delete phost;
    }

    return TRUE;
}
//随便改点什么让git提交上去
void FireEventUI(CRichEditUI *re,CControlUI *ctrl,EVENTTYPE_UI nT,WPARAM wParam)
{
	TEventUI ev;
	ev.pSender = ctrl;
	ev.wParam  = wParam;
	ev.Type    = nT;
	re->DoEvent(ev);
}

CTxtWinHost::CTxtWinHost() : m_re(NULL),m_fScrollThum(1.0),pdoc(NULL),
							_UiState(ResetCmp),_SysCmd(SysEmpty)
{
	m_richedit = NULL;
    ::ZeroMemory(&cRefs, sizeof(CTxtWinHost) - offsetof(CTxtWinHost, cRefs));
	::ZeroMemory(&m_rcInset,sizeof(RECT));

	cchTextMost = cInitTextMax;
    laccelpos = -1;
	bThrowEvent= true;
	ZeroMemory(&szScrollMove,sizeof(SIZE));
}

CTxtWinHost::~CTxtWinHost()
{
	if( pserv )
	{
		pserv->OnTxInPlaceDeactivate();
		pserv->Release();
	}
	if( pdoc )
		pdoc->Release();
	if( m_richedit )
	{
		FreeLibrary(m_richedit);
		m_richedit = NULL;
	}
}

////////////////////// Create/Init/Destruct Commands ///////////////////////

BOOL CTxtWinHost::Init(CRichEditUI *re, const CREATESTRUCT *pcs)
{
    IUnknown *pUnk;
    HRESULT hr;
	
    m_re = re;
    // Initialize Reference count
    cRefs = 1;

    // Create and cache CHARFORMAT for this control
    if(FAILED(InitDefaultCharFormat(re, &cf, NULL)))
        goto err;

    // Create and cache PARAFORMAT for this control
    if(FAILED(InitDefaultParaFormat(re, &pf)))
        goto err;

    // edit controls created without a window are multiline by default
    // so that paragraph formats can be
    dwStyle = ES_MULTILINE;

    // edit controls are rich by default
    fRich = re->IsRich();

    cchTextMost  = re->GetLimitText();
	//缓存滚动条属性
	m_ScrollStyle= 0;
	if( (pcs->style&ES_AUTOHSCROLL)==ES_AUTOHSCROLL )
		m_ScrollStyle |= ES_AUTOHSCROLL;
	if( (pcs->style&ES_AUTOVSCROLL)==ES_AUTOVSCROLL )
		m_ScrollStyle |= ES_AUTOVSCROLL;
	if( (pcs->style&WS_HSCROLL)==WS_HSCROLL )
		m_ScrollStyle |= WS_HSCROLL;
	if( (pcs->style&WS_VSCROLL)==WS_VSCROLL )
		m_ScrollStyle |= WS_VSCROLL;
	if( (pcs->style&ES_DISABLENOSCROLL)==ES_DISABLENOSCROLL )
		m_ScrollStyle |= ES_DISABLENOSCROLL;

    if (pcs )
    {
        dwStyle = pcs->style;

        if ( !(dwStyle & (ES_AUTOHSCROLL | WS_HSCROLL)) )
        {
            fWordWrap = TRUE;
        }
    }

    if( !(dwStyle & ES_LEFT) )
    {
        if(dwStyle & ES_CENTER)
            pf.wAlignment = PFA_CENTER;
        else if(dwStyle & ES_RIGHT)
            pf.wAlignment = PFA_RIGHT;
    }

    fInplaceActive = TRUE;

    // Create Text Services component
	m_richedit = LoadRichEdit(NULL, this, &pUnk);
	if( m_richedit==NULL )
	{
		if(FAILED(CreateTextServices(NULL, this, &pUnk)))
			goto err;
	}
    hr = pUnk->QueryInterface(IID_ITextServices,(void **)&pserv);
	hr = pUnk->QueryInterface(IID_ITextDocument2,(void **)&pdoc);
    // Whether the previous call succeeded or failed we are done
    // with the private interface.
    pUnk->Release();
	if(pserv==NULL || pdoc==NULL)
	{
		goto err;
	}
    // Set window text
    if(pcs && pcs->lpszName)
    {
#ifdef _UNICODE		
        if(FAILED(pserv->TxSetText((TCHAR *)pcs->lpszName)))
            goto err;
#else
        size_t iLen = _tcslen(pcs->lpszName);
        LPWSTR lpText = new WCHAR[iLen + 1];
        ::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
        ::MultiByteToWideChar(CP_ACP, 0, pcs->lpszName, -1, (LPWSTR)lpText, iLen) ;
        if(FAILED(pserv->TxSetText((LPWSTR)lpText))) {
            delete[] lpText;
            goto err;
        }
        delete[] lpText;
#endif
    }

    return TRUE;
err:
    return FALSE;
}

/////////////////////////////////  IUnknown ////////////////////////////////


HRESULT CTxtWinHost::QueryInterface(REFIID riid, void **ppvObject)
{
    HRESULT hr = E_NOINTERFACE;
    *ppvObject = NULL;

    if (IsEqualIID(riid, IID_IUnknown) 
        || IsEqualIID(riid, IID_ITextHost)) 
    {
        AddRef();
        *ppvObject = (ITextHost *) this;
        hr = S_OK;
    }

    return hr;
}

ULONG CTxtWinHost::AddRef(void)
{
    return ++cRefs;
}

ULONG CTxtWinHost::Release(void)
{
    ULONG c_Refs = --cRefs;

    if (c_Refs == 0)
    {
        delete this;
    }

    return c_Refs;
}

/////////////////////////////////  Far East Support  //////////////////////////////////////

HIMC CTxtWinHost::TxImmGetContext(void)
{
	return ImmGetContext(m_re->GetManager()->GetPaintWindow());
}

void CTxtWinHost::TxImmReleaseContext(HIMC himc)
{
	ImmReleaseContext(m_re->GetManager()->GetPaintWindow(),himc);
}

//////////////////////////// ITextHost Interface  ////////////////////////////

HDC CTxtWinHost::TxGetDC()
{
    return m_re->GetManager()->GetPaintDC();
}

int CTxtWinHost::TxReleaseDC(HDC hdc)
{
    return 1;
}

BOOL CTxtWinHost::TxShowScrollBar(INT fnBar, BOOL fShow)
{
	CScrollBarUI* pVerticalScrollBar = m_re->GetVerticalScrollBar();
    CScrollBarUI* pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
    if( fnBar == SB_VERT && pVerticalScrollBar ) 
	{
        
		pVerticalScrollBar->SetVisible(fShow == TRUE);

		FireEventUI(m_re,pVerticalScrollBar,UIEVENT_SCROLL_VISIBLE,pVerticalScrollBar->IsVisible());
    }
    else if( fnBar == SB_HORZ && pHorizontalScrollBar )
	{
        pHorizontalScrollBar->SetVisible(fShow == TRUE);

		FireEventUI(m_re,pHorizontalScrollBar,UIEVENT_SCROLL_VISIBLE,pHorizontalScrollBar->IsVisible());

    }
    else if( fnBar == SB_BOTH ) 
	{
        if( pVerticalScrollBar )
		{
			pVerticalScrollBar->SetVisible(fShow == TRUE);
			
			FireEventUI(m_re,pVerticalScrollBar,UIEVENT_SCROLL_VISIBLE,pVerticalScrollBar->IsVisible());
		}
        if( pHorizontalScrollBar ) 
		{
			pHorizontalScrollBar->SetVisible(fShow == TRUE);

			FireEventUI(m_re,pHorizontalScrollBar,UIEVENT_SCROLL_VISIBLE,pHorizontalScrollBar->IsVisible());
		}
    }
    return TRUE;
}

BOOL CTxtWinHost::TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags)
{
    if( fuSBFlags == SB_VERT ) {
        m_re->EnableScrollBar(true, m_re->GetHorizontalScrollBar() != NULL);
        m_re->GetVerticalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
		
		FireEventUI(m_re,m_re->GetVerticalScrollBar(),UIEVENT_SCROLL_VISIBLE,
					m_re->GetVerticalScrollBar()->IsVisible());
    }
    else if( fuSBFlags == SB_HORZ ) {
        m_re->EnableScrollBar(m_re->GetVerticalScrollBar() != NULL, true);
        m_re->GetHorizontalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);

		FireEventUI(m_re,m_re->GetHorizontalScrollBar(),UIEVENT_SCROLL_VISIBLE,
			m_re->GetHorizontalScrollBar()->IsVisible());
    }
    else if( fuSBFlags == SB_BOTH ) {
        m_re->EnableScrollBar(true, true);
        m_re->GetVerticalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
        m_re->GetHorizontalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);

		FireEventUI(m_re,m_re->GetVerticalScrollBar(),UIEVENT_SCROLL_VISIBLE,
			m_re->GetVerticalScrollBar()->IsVisible());
		FireEventUI(m_re,m_re->GetHorizontalScrollBar(),UIEVENT_SCROLL_VISIBLE,
			m_re->GetHorizontalScrollBar()->IsVisible());
    }
    return TRUE;
}

BOOL CTxtWinHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
    CScrollBarUI* pVerticalScrollBar = m_re->GetVerticalScrollBar();
    CScrollBarUI* pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
	if( fnBar == SB_VERT && pVerticalScrollBar ) 
	{
		bool bOldV = pVerticalScrollBar->IsVisible();
        if( nMaxPos - nMinPos - rcClient.bottom + rcClient.top <= 0 )
		{
            pVerticalScrollBar->SetVisible(false);
        }
        else 
		{
            pVerticalScrollBar->SetVisible(true);
            pVerticalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.bottom + rcClient.top);
		}
		if( bOldV != pVerticalScrollBar->IsVisible() )
		{
			FireEventUI(m_re,m_re->GetVerticalScrollBar(),UIEVENT_SCROLL_VISIBLE,
				m_re->GetVerticalScrollBar()->IsVisible());
		}
    }
    else if( fnBar == SB_HORZ && pHorizontalScrollBar )
	{
		bool bOldH = pHorizontalScrollBar->IsVisible();

        if( nMaxPos - nMinPos - rcClient.right + rcClient.left <= 0 ) 
		{
            pHorizontalScrollBar->SetVisible(false);
        }
        else 
		{
            pHorizontalScrollBar->SetVisible(true);
            pHorizontalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.right + rcClient.left);
		}
		if( bOldH != pHorizontalScrollBar->IsVisible()  )
		{
			FireEventUI(m_re,m_re->GetHorizontalScrollBar(),UIEVENT_SCROLL_VISIBLE,
				m_re->GetHorizontalScrollBar()->IsVisible());
		}
    }
    return TRUE;
}

BOOL CTxtWinHost::TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw)
{
    CScrollBarUI* pVerticalScrollBar = m_re->GetVerticalScrollBar();
    CScrollBarUI* pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
	
	CRichEditUI::IN_MSG	param;
	param.nMsg = CRichEditUI::SCROLL_MovePos;
	param.nBarType = fnBar;
	
	if( fnBar == SB_VERT && pVerticalScrollBar ) 
	{

		param.nMovePos = nPos - pVerticalScrollBar->GetScrollPos();
		szScrollMove.cy = param.nMovePos;

		pVerticalScrollBar->SetScrollPos(nPos);
		
		if( bThrowEvent )
		{
			IRichEditOle* pReOle = NULL;
			m_re->TxSendMessage( EM_GETOLEINTERFACE, 0, (LPARAM)&pReOle, 0);
			FireOleScroll(pReOle,0,param.nMovePos);
			if( pReOle )
				pReOle->Release();
			m_re->OnScrollBar( (void*)(&param) );
		}
	}
    else if( fnBar == SB_HORZ && pHorizontalScrollBar )
	{
        pHorizontalScrollBar->SetScrollPos(nPos);

		param.nMovePos = nPos;
		if( bThrowEvent )
			m_re->OnScrollBar( (void*)(&param) );
	}
    return TRUE;
}

void CTxtWinHost::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
    if( prc == NULL ) {
        m_re->GetManager()->Invalidate(rcClient);
        return;
    }
    RECT rc = *prc;
    m_re->GetManager()->Invalidate(rc);
}

void CTxtWinHost::TxViewChange(BOOL fUpdate) 
{
    if( m_re->OnTxViewChanged() )
		m_re->Invalidate();
}

BOOL CTxtWinHost::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
	m_CaretxWidth = xWidth;
	m_CaretyHeight= yHeight;
	return ::CreateCaret(m_re->GetManager()->GetPaintWindow(), hbmp, xWidth, yHeight);
}

BOOL CTxtWinHost::TxShowCaret(BOOL fShow)
{
	if( m_re->IsHideCaret() )
		return FALSE;
	if(fShow)
        return ::ShowCaret(m_re->GetManager()->GetPaintWindow());
    else
        return ::HideCaret(m_re->GetManager()->GetPaintWindow());
}

BOOL CTxtWinHost::TxSetCaretPos(INT x, INT y)
{
	BOOL bRet = FALSE;
	if( GetReadOnly() && 0==m_re->GetTextLength() )
	{
		return bRet;
	}
	bRet = ::SetCaretPos(x, y);
	if( bRet )
	{
		RECT rc={x,y,x+m_CaretxWidth,y+m_CaretyHeight};
	}
	return bRet;
}

BOOL CTxtWinHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
    fTimer = TRUE;
    return m_re->GetManager()->SetTimer(m_re, idTimer, uTimeout) == TRUE;
}

void CTxtWinHost::TxKillTimer(UINT idTimer)
{
    m_re->GetManager()->KillTimer(m_re, idTimer);
    fTimer = FALSE;
}

void CTxtWinHost::TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll,	LPCRECT lprcClip,	HRGN hrgnUpdate, LPRECT lprcUpdate,	UINT fuScroll)	
{
	int n = 0;
	n++;
    return;
}

void CTxtWinHost::TxSetCapture(BOOL fCapture)
{
    if (fCapture) m_re->GetManager()->SetCapture();
    else m_re->GetManager()->ReleaseCapture();
    fCaptured = fCapture;
}

void CTxtWinHost::TxSetFocus()
{
    m_re->SetFocus();
}

void CTxtWinHost::TxSetCursor(HCURSOR hcur,	BOOL fText)
{
    ::SetCursor(hcur);
}

BOOL CTxtWinHost::TxScreenToClient(LPPOINT lppt)
{
    return ::ScreenToClient(m_re->GetManager()->GetPaintWindow(), lppt);	
}

BOOL CTxtWinHost::TxClientToScreen(LPPOINT lppt)
{
    return ::ClientToScreen(m_re->GetManager()->GetPaintWindow(), lppt);
}

HRESULT CTxtWinHost::TxActivate(LONG *plOldState)
{
    return S_OK;
}

HRESULT CTxtWinHost::TxDeactivate(LONG lNewState)
{
    return S_OK;
}

HRESULT CTxtWinHost::TxGetClientRect(LPRECT prc)
{
	//mark
    *prc = rcClient;
    GetControlRect(prc);
	return S_OK;
}

HRESULT CTxtWinHost::TxGetViewInset(LPRECT prc) 
{
	//prc->left = prc->right = prc->top = prc->bottom = 0;
	*prc = m_rcInset;
	return S_OK;
}

HRESULT CTxtWinHost::TxGetCharFormat(const CHARFORMATW **ppCF)
{
    *ppCF = &cf;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetParaFormat(const PARAFORMAT **ppPF)
{
    *ppPF = &pf;
    return NOERROR;
}

COLORREF CTxtWinHost::TxGetSysColor(int nIndex) 
{
    return ::GetSysColor(nIndex);
}

HRESULT CTxtWinHost::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
    *pstyle = !fTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetMaxLength(DWORD *pLength)
{
    *pLength = cchTextMost;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetScrollBars(DWORD *pdwScrollBar)
{
    //*pdwScrollBar =  dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | 
    //    ES_AUTOHSCROLL | ES_DISABLENOSCROLL);
	*pdwScrollBar = m_ScrollStyle;
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetPasswordChar(TCHAR *pch)
{
#ifdef _UNICODE
    *pch = chPasswordChar;
#else
    ::WideCharToMultiByte(CP_ACP, 0, &chPasswordChar, 1, pch, 1, NULL, NULL) ;
#endif
    return NOERROR;
}

HRESULT CTxtWinHost::TxGetAcceleratorPos(LONG *pcp)
{
    *pcp = laccelpos;
    return S_OK;
} 										   

HRESULT CTxtWinHost::OnTxCharFormatChange(const CHARFORMATW *pcf)
{
    return S_OK;
}

HRESULT CTxtWinHost::OnTxParaFormatChange(const PARAFORMAT *ppf)
{
    return S_OK;
}

HRESULT CTxtWinHost::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) 
{
    DWORD dwProperties = 0;

    if (fRich)
    {
        dwProperties = TXTBIT_RICHTEXT;
    }

    if (dwStyle & ES_MULTILINE)
    {
        dwProperties |= TXTBIT_MULTILINE;
    }

    if (dwStyle & ES_READONLY)
    {
        dwProperties |= TXTBIT_READONLY;
    }

    if (dwStyle & ES_PASSWORD)
    {
        dwProperties |= TXTBIT_USEPASSWORD;
    }

    if (!(dwStyle & ES_NOHIDESEL))
    {
        dwProperties |= TXTBIT_HIDESELECTION;
    }

    if (fEnableAutoWordSel)
    {
       dwProperties |= TXTBIT_AUTOWORDSEL;
    }

    if (fWordWrap)
    {
        dwProperties |= TXTBIT_WORDWRAP;
    }

    if (fAllowBeep)
    {
        dwProperties |= TXTBIT_ALLOWBEEP;
    }

    if (fSaveSelection)
    {
        dwProperties |= TXTBIT_SAVESELECTION;
    }

    *pdwBits = dwProperties & dwMask; 
    return NOERROR;
}


HRESULT CTxtWinHost::TxNotify(DWORD iNotify, void *pv)
{
    if( iNotify == EN_REQUESTRESIZE )
	{
        RECT rc;
        REQRESIZE *preqsz = (REQRESIZE *)pv;
        GetControlRect(&rc);
        rc.bottom = rc.top + preqsz->rc.bottom;
        rc.right  = rc.left + preqsz->rc.right;
  	}
    m_re->OnTxNotify(iNotify, pv);
    return S_OK;
}

HRESULT CTxtWinHost::TxGetExtent(LPSIZEL lpExtent)
{
    *lpExtent = sizelExtent;
    return S_OK;
}

HRESULT	CTxtWinHost::TxGetSelectionBarWidth (LONG *plSelBarWidth)
{
    *plSelBarWidth = lSelBarWidth;
    return S_OK;
}
void CTxtWinHost::EnableScroll(DWORD dwScroll,BOOL bEnable)
{
	if( dwScroll == SB_BOTH )
	{
		if( bEnable==false )
		{
			m_ScrollStyle &= ~ES_AUTOHSCROLL;
			m_ScrollStyle &= ~WS_HSCROLL;

			m_ScrollStyle &= ~ES_AUTOVSCROLL;
			m_ScrollStyle &= ~WS_VSCROLL;
		}
		else
		{
			m_ScrollStyle |= ES_AUTOHSCROLL;
			m_ScrollStyle |= WS_HSCROLL;

			m_ScrollStyle |= ES_AUTOVSCROLL;
			m_ScrollStyle |= WS_VSCROLL;
		}
	}
	if( dwScroll == SB_VERT )
	{
		if( bEnable==false )
		{
			m_ScrollStyle &= ~ES_AUTOVSCROLL;
			m_ScrollStyle &= ~WS_VSCROLL;
		}
		else
		{
			m_ScrollStyle |= ES_AUTOVSCROLL;
			m_ScrollStyle |= WS_VSCROLL;
		}
	}
	if( dwScroll == SB_HORZ )
	{
		if( bEnable==false )
		{
			m_ScrollStyle &= ~ES_AUTOHSCROLL;
			m_ScrollStyle &= ~WS_HSCROLL;
		}
		else
		{
			m_ScrollStyle |= ES_AUTOHSCROLL;
			m_ScrollStyle |= WS_HSCROLL;
		}
	}
	pserv->OnTxPropertyBitsChange(TXTBIT_SCROLLBARCHANGE,TXTBIT_SCROLLBARCHANGE);
}
void CTxtWinHost::SetWordWrap(BOOL fWordWrap)
{
    pserv->OnTxPropertyBitsChange(TXTBIT_WORDWRAP, fWordWrap ? TXTBIT_WORDWRAP : 0);
}

BOOL CTxtWinHost::GetReadOnly()
{
    return (dwStyle & ES_READONLY) != 0;
}

void CTxtWinHost::SetReadOnly(BOOL fReadOnly)
{
    if (fReadOnly)
    {
        dwStyle |= ES_READONLY;
    }
    else
    {
        dwStyle &= ~ES_READONLY;
    }

    pserv->OnTxPropertyBitsChange(TXTBIT_READONLY, 
        fReadOnly ? TXTBIT_READONLY : 0);
}

void CTxtWinHost::SetFont(HFONT hFont) 
{
	//fixed by jiayh
    if( hFont == NULL ) return;
    LOGFONT lf;
    ::GetObject(hFont, sizeof(LOGFONT), &lf);
    LONG yPixPerInch = ::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSY);
    cf.yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
    if(lf.lfWeight >= FW_BOLD)
        cf.dwEffects |= CFE_BOLD;
	else
		 cf.dwEffects &= ~CFE_BOLD;
    if(lf.lfItalic)
        cf.dwEffects |= CFE_ITALIC;
	else
		cf.dwEffects &= ~CFE_ITALIC;

	if(lf.lfUnderline)
        cf.dwEffects |= CFE_UNDERLINE;
	else
		cf.dwEffects &= ~CFE_UNDERLINE;
    cf.bCharSet = lf.lfCharSet;
    cf.bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
    _tcscpy(cf.szFaceName, lf.lfFaceName);
#else
    //need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
    MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, cf.szFaceName, LF_FACESIZE) ;
#endif

    pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
        TXTBIT_CHARFORMATCHANGE);
}

void CTxtWinHost::SetColor(DWORD dwColor)
{
    cf.crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
    pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
        TXTBIT_CHARFORMATCHANGE);
}

SIZEL* CTxtWinHost::GetExtent() 
{
    return &sizelExtent;
}

void CTxtWinHost::SetExtent(SIZEL *psizelExtent) 
{ 
    sizelExtent = *psizelExtent; 
    pserv->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
}

void CTxtWinHost::LimitText(LONG nChars)
{
    cchTextMost = nChars;
    if( cchTextMost <= 0 ) cchTextMost = cInitTextMax;
	pserv->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, TXTBIT_MAXLENGTHCHANGE);
}

BOOL CTxtWinHost::IsCaptured()
{
    return fCaptured;
}

BOOL CTxtWinHost::GetAllowBeep()
{
    return fAllowBeep;
}

void CTxtWinHost::SetAllowBeep(BOOL fAllowBeep)
{
    pserv->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, 
        fAllowBeep ? TXTBIT_ALLOWBEEP : 0);
}

WORD CTxtWinHost::GetDefaultAlign()
{
    return pf.wAlignment;
}

void CTxtWinHost::SetDefaultAlign(WORD wNewAlign)
{
    pf.wAlignment = wNewAlign;

    // Notify control of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

BOOL CTxtWinHost::GetRichTextFlag()
{
    return fRich;
}

void CTxtWinHost::SetRichTextFlag(BOOL fNew)
{
    fRich = fNew;

    pserv->OnTxPropertyBitsChange(TXTBIT_RICHTEXT, 
        fNew ? TXTBIT_RICHTEXT : 0);
}

LONG CTxtWinHost::GetDefaultLeftIndent()
{
    return pf.dxOffset;
}

void CTxtWinHost::SetDefaultLeftIndent(LONG lNewIndent)
{
    pf.dxOffset = lNewIndent;

    pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

void CTxtWinHost::SetClientRect(RECT *prc) 
{
	if( !::EqualRect(&rcClient,prc) )
	{
		_UiState  = ResizedRc;
	}
	rcClient = *prc;
    LONG xPerInch = ::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSX); 
    LONG yPerInch =	::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSY); 
	sizelExtent.cx = DtoHimetric(rcClient.right - rcClient.left, xPerInch);
	sizelExtent.cy = DtoHimetric(rcClient.bottom - rcClient.top, yPerInch);
    pserv->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE|TXTBIT_CLIENTRECTCHANGE|TXTBIT_VIEWINSETCHANGE,TXTBIT_EXTENTCHANGE|TXTBIT_CLIENTRECTCHANGE|TXTBIT_VIEWINSETCHANGE);
}
void CTxtWinHost::NotifyDrawItem()
{
	if( m_re == NULL || _UiState==ResetCmp )
		return ;
	RECT rcContain = m_re->GetPos();
	if( _UiState == UpdateItems )
	{
		_UiState = ResetCmp;
		for( int it = 0; it < m_re->GetCount(); it++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_re->GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) 
			{
				m_re->SetFloatPos(it);
			}
			else 
			{
				pControl->SetPos(rcContain); // 所有非float子控件放大到整个客户区
			}
		}
		m_re->Invalidate();
		return ;
	}
	if( _UiState == ReDrawItem )
	{
		_UiState = ResetCmp;
		for( int it = 0; it < m_re->GetCount(); it++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_re->GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) 
			{
				m_re->SetFloatPos(it);
			}
			else 
			{
				pControl->SetPos(rcContain); // 所有非float子控件放大到整个客户区
			}
		}
		m_re->Invalidate();
		return ;
	}
	if( _UiState == ResizedRc )
	{
		for( int it = 0; it < m_re->GetCount(); it++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_re->GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) 
			{
				m_re->SetFloatPos(it);
			}
			else 
			{
				pControl->SetPos(rcContain); // 所有非float子控件放大到整个客户区
			}
		}
		//因为控件内部会调用setsel，设置布局方式从而影响滚动条定位因此需要调整调用顺序
		if( m_re->GetVerticalScrollBar() && m_re->GetVerticalScrollBar()->IsVisible() )
		{
			if(m_fScrollThum==1.0)
			{
				m_re->TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L, 0);
			}
			else
			{
				int nOldThum = m_re->GetVerticalScrollBar()->GetScrollRange()*m_fScrollThum;
				WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION,nOldThum);
				m_re->TxSendMessage(WM_VSCROLL, wParam, 0L, 0);
			}	
		}
		_UiState = ResetScoll;
		m_re->Invalidate();
		return ;
	}
	if( _UiState == ResetScoll )
	{
		_UiState = ResetCmp;
		for( int it = 0; it < m_re->GetCount(); it++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_re->GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) 
			{
				m_re->SetFloatPos(it);
			}
			else 
			{
				pControl->SetPos(rcContain); // 所有非float子控件放大到整个客户区
			}
		}
		m_re->Invalidate();
		return ;
	}
}
void CTxtWinHost::UpdateInset() 
{
	LONG xPerInch = ::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSX); 
	LONG yPerInch =	::GetDeviceCaps(m_re->GetManager()->GetPaintDC(), LOGPIXELSY); 
	m_rcInset.left  = DtoHimetric(m_re->GetInset().left ,xPerInch);
	m_rcInset.right = DtoHimetric(m_re->GetInset().right,xPerInch);
	m_rcInset.top   = DtoHimetric(m_re->GetInset().top,yPerInch);
	m_rcInset.bottom= DtoHimetric(m_re->GetInset().bottom,yPerInch);
	pserv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE,TXTBIT_VIEWINSETCHANGE);
}

BOOL CTxtWinHost::SetSaveSelection(BOOL f_SaveSelection)
{
    BOOL fResult = f_SaveSelection;

    fSaveSelection = f_SaveSelection;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_SAVESELECTION, 
        fSaveSelection ? TXTBIT_SAVESELECTION : 0);

    return fResult;		
}

HRESULT	CTxtWinHost::OnTxInPlaceDeactivate()
{
    HRESULT hr = pserv->OnTxInPlaceDeactivate();

    if (SUCCEEDED(hr))
    {
        fInplaceActive = FALSE;
    }

    return hr;
}

HRESULT	CTxtWinHost::OnTxInPlaceActivate(LPCRECT prcClient)
{
    fInplaceActive = TRUE;

    HRESULT hr = pserv->OnTxInPlaceActivate(prcClient);

    if (FAILED(hr))
    {
        fInplaceActive = FALSE;
    }

    return hr;
}

BOOL CTxtWinHost::DoSetCursor(RECT *prc, POINT *pt)
{
    RECT rc = prc ? *prc : rcClient;

    // Is this in our rectangle?
    if (PtInRect(&rc, *pt))
    {
        RECT *prcClient = (!fInplaceActive || prc) ? &rc : NULL;
        pserv->OnTxSetCursor(DVASPECT_CONTENT,	-1, NULL, NULL,  m_re->GetManager()->GetPaintDC(),
            NULL, prcClient, pt->x, pt->y);

        return TRUE;
    }

    return FALSE;
}

void CTxtWinHost::GetControlRect(LPRECT prc)
{
    prc->top = rcClient.top;
    prc->bottom = rcClient.bottom;
    prc->left = rcClient.left;
    prc->right = rcClient.right;
}

void CTxtWinHost::SetTransparent(BOOL f_Transparent)
{
    fTransparent = f_Transparent;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
}

LONG CTxtWinHost::SetAccelPos(LONG l_accelpos)
{
    LONG laccelposOld = l_accelpos;

    laccelpos = l_accelpos;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_SHOWACCELERATOR, 0);

    return laccelposOld;
}

WCHAR CTxtWinHost::SetPasswordChar(WCHAR ch_PasswordChar)
{
    WCHAR chOldPasswordChar = chPasswordChar;

    chPasswordChar = ch_PasswordChar;

    // notify text services of property change
    pserv->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD, 
        (chPasswordChar != 0) ? TXTBIT_USEPASSWORD : 0);

    return chOldPasswordChar;
}

void CTxtWinHost::SetDisabled(BOOL fOn)
{
    cf.dwMask	 |= CFM_COLOR | CFM_DISABLED;
    cf.dwEffects |= CFE_AUTOCOLOR | CFE_DISABLED;

    if( !fOn )
    {
        cf.dwEffects &= ~CFE_DISABLED;
    }

    pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
        TXTBIT_CHARFORMATCHANGE);
}

LONG CTxtWinHost::SetSelBarWidth(LONG l_SelBarWidth)
{
    LONG lOldSelBarWidth = lSelBarWidth;

    lSelBarWidth = l_SelBarWidth;

    if (lSelBarWidth)
    {
        dwStyle |= ES_SELECTIONBAR;
    }
    else
    {
        dwStyle &= (~ES_SELECTIONBAR);
    }

    pserv->OnTxPropertyBitsChange(TXTBIT_SELBARCHANGE, TXTBIT_SELBARCHANGE);

    return lOldSelBarWidth;
}

BOOL CTxtWinHost::GetTimerState()
{
    return fTimer;
}

void CTxtWinHost::SetCharFormat(CHARFORMAT2W &c)
{
    cf = c;
}

void CTxtWinHost::SetParaFormat(PARAFORMAT2 &p)
{
    pf = p;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

bool HandleLimitText(CRichEditUI *rich,long nInsertAfterChar, LPCTSTR lpstrText, bool bCanUndo)
{
	if( nInsertAfterChar == -1 )
		nInsertAfterChar = rich->GetTextLength();
	if( rich->GetLimitText() != cInitTextMax )
	{
		long nTotalMax = rich->GetTextLength();
		if( nTotalMax >= rich->GetLimitText() )
		{
			return true;
		}
		if( nTotalMax == nInsertAfterChar )
		{
			UINT nChars = rich->GetLimitText()-nTotalMax;
			if( nChars < _tcsclen(lpstrText) )
			{
				CDuiString strTxt(lpstrText,nChars);
				rich->ReplaceSel(strTxt, bCanUndo);
				return true;
			}
		}
	}
	return false;
}

CRichEditUI::CRichEditUI() 
	: m_pTwh(NULL), m_bVScrollBarFixing(false), m_bWantTab(true), m_bWantReturn(true), 
    m_bWantCtrlReturn(true), m_bRich(true), m_bReadOnly(false), m_bWordWrap(false), 
	m_dwTextColor(0),m_iFont(-1), m_iLimitText(cInitTextMax), m_lTwhStyle(ES_MULTILINE),
	m_bInited(false),m_bHideCaret(false)
{
	m_bViewChanged = true;
	m_chLeadByte   = 0;
#ifndef _UNICODE
	m_fAccumulateDBC =true;
#else
	m_fAccumulateDBC= false;
#endif
}

CRichEditUI::~CRichEditUI()
{
	OnScrollBar.clear();
	OnChangeRc.clear();
    if( m_pTwh )
	{
        m_pTwh->Release();
        m_pManager->RemoveMessageFilter(this);
    }
}
ITextRange2*	CRichEditUI::GetRange(long nStartChar, long nEndChar)
{
	ITextRange2* pRange = NULL;
	if( m_pTwh && m_pTwh->GetTextDocument() )
	{
		m_pTwh->GetTextDocument()->Range2(nStartChar,nEndChar,&pRange);
	}
	return pRange;
}
bool CRichEditUI::Add(CControlUI* pControl)
{
	bool bRet = CContainerUI::Add(pControl);
	if( m_pTwh )
		m_pTwh->_UiState = CTxtWinHost::ReDrawItem;
	return bRet;
}
bool CRichEditUI::AddAt(CControlUI* pControl, int iIndex)
{
	bool bRet = CContainerUI::AddAt(pControl,iIndex);
	if( m_pTwh )
		m_pTwh->_UiState = CTxtWinHost::ReDrawItem;
	return bRet;
}

void CRichEditUI::UpdateItems()
{
	if( m_pTwh )
	{
		m_pTwh->_UiState = CTxtWinHost::UpdateItems;
	}
	Invalidate();
}
bool CRichEditUI::IsHideCaret()
{
	return m_bHideCaret;
}
void CRichEditUI::HideCaret(bool bHide)
{
	m_bHideCaret = bHide;
	Invalidate();
}
void CRichEditUI::GetCaretSize(SIZE &sz)
{
	if(m_pTwh){
		sz.cx = m_pTwh->m_CaretxWidth;
		sz.cy = m_pTwh->m_CaretyHeight;
	}
}
LPCTSTR CRichEditUI::GetClass() const
{
    return _T("RichEditUI");
}

LPVOID CRichEditUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, DUI_CTR_RICHEDIT) == 0 ) return static_cast<CRichEditUI*>(this);
    return CContainerUI::GetInterface(pstrName);
}

UINT CRichEditUI::GetControlFlags() const
{
    if( !IsEnabled() ) return CControlUI::GetControlFlags();

    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

bool CRichEditUI::IsWantTab()
{
    return m_bWantTab;
}

void CRichEditUI::SetWantTab(bool bWantTab)
{
    m_bWantTab = bWantTab;
}

bool CRichEditUI::IsWantReturn()
{
    return m_bWantReturn;
}

void CRichEditUI::SetWantReturn(bool bWantReturn)
{
    m_bWantReturn = bWantReturn;
}

bool CRichEditUI::IsWantCtrlReturn()
{
    return m_bWantCtrlReturn;
}

void CRichEditUI::SetWantCtrlReturn(bool bWantCtrlReturn)
{
    m_bWantCtrlReturn = bWantCtrlReturn;
}

bool CRichEditUI::IsRich()
{
    return m_bRich;
}

void CRichEditUI::SetRich(bool bRich)
{
    m_bRich = bRich;
    if( m_pTwh ) m_pTwh->SetRichTextFlag(bRich);
}

bool CRichEditUI::IsReadOnly()
{
    return m_bReadOnly;
}

void CRichEditUI::SetReadOnly(bool bReadOnly)
{
    m_bReadOnly = bReadOnly;
    if( m_pTwh ) m_pTwh->SetReadOnly(bReadOnly);
}

bool CRichEditUI::GetWordWrap()
{
    return m_bWordWrap;
}

void CRichEditUI::SetWordWrap(bool bWordWrap)
{
    m_bWordWrap = bWordWrap;
    if( m_pTwh ) m_pTwh->SetWordWrap(bWordWrap);
}

int CRichEditUI::GetFont()
{
    return m_iFont;
}

void CRichEditUI::SetFont(int index)
{
    m_iFont = index;
    if( m_pTwh ) {
        m_pTwh->SetFont(GetManager()->GetFont(m_iFont));
    }
}

void CRichEditUI::SetFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
{
	//fixed by jiayh
    if( m_pTwh ) {
        LOGFONT lf = { 0 };
        ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
        _tcsncpy(lf.lfFaceName, pStrFontName, LF_FACESIZE);
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfHeight = -nSize;
        if( bBold ) 
			lf.lfWeight += FW_BOLD;
		else if( lf.lfWeight >= FW_BOLD )
			lf.lfWeight -= FW_BOLD;
        if( bUnderline ) 
			lf.lfUnderline = TRUE;
		else
			lf.lfUnderline = FALSE;
        if( bItalic )
			lf.lfItalic = TRUE;
		else
			lf.lfItalic = FALSE;
		HFONT hFont = ::CreateFontIndirect(&lf);
        if( hFont == NULL ) return;
        m_pTwh->SetFont(hFont);
        ::DeleteObject(hFont);
    }
}

LONG CRichEditUI::GetWinStyle()
{
    return m_lTwhStyle;
}

void CRichEditUI::SetWinStyle(LONG lStyle)
{
    m_lTwhStyle = lStyle;
}

DWORD CRichEditUI::GetTextColor()
{
    return m_dwTextColor;
}

void CRichEditUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
    if( m_pTwh ) {
        m_pTwh->SetColor(dwTextColor);
    }
}

int CRichEditUI::GetLimitText()
{
    return m_iLimitText;
}

void CRichEditUI::SetLimitText(int iChars)
{
    m_iLimitText = iChars;
    if( m_pTwh ) {
        m_pTwh->LimitText(m_iLimitText);
    }
}

long CRichEditUI::GetTextLength(DWORD dwFlags) const
{
    GETTEXTLENGTHEX textLenEx;
    textLenEx.flags = dwFlags;
#ifdef _UNICODE
    textLenEx.codepage = 1200;
#else
    textLenEx.codepage = CP_ACP;
#endif
    LRESULT lResult;
    TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&textLenEx, 0, &lResult);
    return (long)lResult;
}

CDuiString CRichEditUI::GetText() const
{
    long lLen = GetTextLength(GTL_DEFAULT);
    LPTSTR lpText = NULL;
    GETTEXTEX gt;
    gt.flags = GT_DEFAULT;
#ifdef _UNICODE
    gt.cb = sizeof(TCHAR) * (lLen + 1) ;
    gt.codepage = 1200;
    lpText = new TCHAR[lLen + 1];
    ::ZeroMemory(lpText, (lLen + 1) * sizeof(TCHAR));
#else
    gt.cb = sizeof(TCHAR) * lLen * 2 + 1;
    gt.codepage = CP_ACP;
    lpText = new TCHAR[lLen * 2 + 1];
    ::ZeroMemory(lpText, (lLen * 2 + 1) * sizeof(TCHAR));
#endif
    gt.lpDefaultChar = NULL;
    gt.lpUsedDefChar = NULL;
    TxSendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)lpText, 0);
    CDuiString sText(lpText);
    delete[] lpText;
    return sText;
}

void CRichEditUI::SetText(LPCTSTR pstrText)
{
    m_sText = pstrText;
    if( !m_pTwh ) return;
    SetSel(0, -1);
    ReplaceSel(pstrText, FALSE);
}

bool CRichEditUI::GetModify() const
{ 
    if( !m_pTwh ) return false;
    LRESULT lResult;
    TxSendMessage(EM_GETMODIFY, 0, 0, &lResult);
    return (BOOL)lResult == TRUE;
}

void CRichEditUI::SetModify(bool bModified) const
{ 
    TxSendMessage(EM_SETMODIFY, bModified, 0, 0);
}

void CRichEditUI::GetSel(CHARRANGE &cr) const
{ 
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0); 
}

void CRichEditUI::GetSel(long& nStartChar, long& nEndChar) const
{
    CHARRANGE cr;
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0); 
    nStartChar = cr.cpMin;
    nEndChar = cr.cpMax;
}

int CRichEditUI::SetSel(CHARRANGE &cr)
{ 
    LRESULT lResult;
    TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr, &lResult); 
    return (int)lResult;
}

int CRichEditUI::SetSel(long nStartChar, long nEndChar)
{
    CHARRANGE cr;
    cr.cpMin = nStartChar;
    cr.cpMax = nEndChar;
    LRESULT lResult;
    TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr, &lResult); 
	return (int)lResult;
}

void CRichEditUI::ReplaceSel(LPCTSTR lpszNewText, bool bCanUndo)
{
#ifdef _UNICODE		
	TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpszNewText, 0); 
#else
	int iLen = _tcslen(lpszNewText);
	LPWSTR lpText = new WCHAR[iLen + 1];
	::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
	::MultiByteToWideChar(CP_ACP, 0, lpszNewText, -1, (LPWSTR)lpText, iLen) ;
	TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpText, 0); 
	delete[] lpText;
#endif
}

void CRichEditUI::ReplaceSelW(LPCWSTR lpszNewText, bool bCanUndo)
{
    TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpszNewText, 0); 
}

CDuiString CRichEditUI::GetSelText() const
{
    if( !m_pTwh ) return CDuiString();
    CHARRANGE cr;
    cr.cpMin = cr.cpMax = 0;
    TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0);
    LPWSTR lpText = NULL;
    lpText = new WCHAR[cr.cpMax - cr.cpMin + 1];
    ::ZeroMemory(lpText, (cr.cpMax - cr.cpMin + 1) * sizeof(WCHAR));
    TxSendMessage(EM_GETSELTEXT, 0, (LPARAM)lpText, 0);
    CDuiString sText;
    sText = (LPCWSTR)lpText;
    delete[] lpText;
    return sText;
}

int CRichEditUI::SetSelAll()
{
    return SetSel(0, -1);
}

int CRichEditUI::SetSelNone()
{
    return SetSel(-1, 0);
}

bool CRichEditUI::GetZoom(int& nNum, int& nDen) const
{
    LRESULT lResult;
    TxSendMessage(EM_GETZOOM, (WPARAM)&nNum, (LPARAM)&nDen, &lResult);
    return (BOOL)lResult == TRUE;
}

bool CRichEditUI::SetZoom(int nNum, int nDen)
{
    if (nNum < 0 || nNum > 64) return false;
    if (nDen < 0 || nDen > 64) return false;
    LRESULT lResult;
    TxSendMessage(EM_SETZOOM, nNum, nDen, &lResult);
    return (BOOL)lResult == TRUE;
}

bool CRichEditUI::SetZoomOff()
{
    LRESULT lResult;
    TxSendMessage(EM_SETZOOM, 0, 0, &lResult);
    return (BOOL)lResult == TRUE;
}

WORD CRichEditUI::GetSelectionType() const
{
    LRESULT lResult;
    TxSendMessage(EM_SELECTIONTYPE, 0, 0, &lResult);
    return (WORD)lResult;
}

bool CRichEditUI::GetAutoURLDetect() const
{
    LRESULT lResult;
    TxSendMessage(EM_GETAUTOURLDETECT, 0, 0, &lResult);
    return (BOOL)lResult == TRUE;
}

bool CRichEditUI::SetAutoURLDetect(bool bAutoDetect)
{
    LRESULT lResult;
    TxSendMessage(EM_AUTOURLDETECT, bAutoDetect, 0, &lResult);
    return (BOOL)lResult == FALSE;
}

DWORD CRichEditUI::GetEventMask() const
{
    LRESULT lResult = 0;
    TxSendMessage(EM_GETEVENTMASK, 0, 0, &lResult);
    return (DWORD)lResult;
}

DWORD CRichEditUI::SetEventMask(DWORD dwEventMask)
{
    LRESULT lResult = 0;
    TxSendMessage(EM_SETEVENTMASK, 0, dwEventMask, &lResult);
    return (DWORD)lResult;
}

CDuiString CRichEditUI::GetTextRange(long nStartChar, long nEndChar) const
{
    TEXTRANGEW tr = { 0 };
    tr.chrg.cpMin = nStartChar;
    tr.chrg.cpMax = nEndChar;
    LPWSTR lpText = NULL;
    lpText = new WCHAR[nEndChar - nStartChar + 1];
    ::ZeroMemory(lpText, (nEndChar - nStartChar + 1) * sizeof(WCHAR));
    tr.lpstrText = lpText;
    TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr, 0);
    CDuiString sText;
    sText = (LPCWSTR)lpText;
    delete[] lpText;
    return sText;
}

void CRichEditUI::HideSelection(bool bHide, bool bChangeStyle)
{
    TxSendMessage(EM_HIDESELECTION, bHide, bChangeStyle, 0);
}

void CRichEditUI::ScrollCaret()
{
    TxSendMessage(EM_SCROLLCARET, 0, 0, 0);
}

int CRichEditUI::InsertText(long nInsertAfterChar, LPCTSTR lpstrText, bool bCanUndo)
{
    /*int nRet = SetSel(nInsertAfterChar, nInsertAfterChar);
	if( HandleLimitText(this,nInsertAfterChar,lpstrText,bCanUndo) )
	{
		return nRet;
	}
    ReplaceSel(lpstrText, bCanUndo);
    return nRet;*/
	long nRet = 0;
	//改成无效是为了贴入文本不影响输入焦点
	if( m_pTwh )
		m_pTwh->GetTextServices()->OnTxUIDeactivate();
	ITextRange2 *rg = GetRange(0,0);
	BOOL bRead = m_pTwh->GetReadOnly();
	if( bRead )
		m_pTwh->SetReadOnly(FALSE);
	rg->Move(tomCharacter,nInsertAfterChar,NULL);
	rg->SetText(CComBSTR(lpstrText));
	rg->GetStoryLength(&nRet);
	rg->Release();    
	if( bRead )
		m_pTwh->SetReadOnly(TRUE);
	return nRet;
}

int CRichEditUI::AppendText(LPCTSTR lpstrText, bool bCanUndo)
{
    long nRet = 0;
	long nCurLen = GetTextLength();
	//改成无效是为了贴入文本不影响输入焦点
	if( m_pTwh )
		m_pTwh->GetTextServices()->OnTxUIDeactivate();
	ITextRange2 *rg = GetRange(0,0);
	BOOL bRead = m_pTwh->GetReadOnly();
	if( bRead )
		m_pTwh->SetReadOnly(FALSE);
	rg->Move(tomCharacter,nCurLen,NULL);
	rg->SetText(CComBSTR(lpstrText));
	rg->GetStoryLength(&nRet);
	rg->Release();    
	if( bRead )
		m_pTwh->SetReadOnly(TRUE);
	return nRet;
}
int CRichEditUI::DeleteText(long nStartChar, long nEndChar)
{
	long nRet = 0;
	long nCurLen = GetTextLength();
	if( m_pTwh )
		m_pTwh->GetTextServices()->OnTxUIDeactivate();
	ITextRange2 *rg = GetRange(nStartChar,nEndChar);
	BOOL bRead = m_pTwh->GetReadOnly();
	if( bRead )
		m_pTwh->SetReadOnly(FALSE);
	rg->Delete(tomCharacter,0,NULL);
	rg->Release();    
	if( bRead )
		m_pTwh->SetReadOnly(TRUE);
	nRet = GetTextLength();
	return nRet;
}
DWORD CRichEditUI::GetDefaultCharFormat(CHARFORMAT2 &cf) const
{
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf, &lResult);
    return (DWORD)lResult;
}

bool CRichEditUI::SetDefaultCharFormat(CHARFORMAT2 &cf)
{
    if( !m_pTwh ) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, 0, (LPARAM)&cf, &lResult);
    if( (BOOL)lResult == TRUE ) {
        CHARFORMAT2W cfw;
        cfw.cbSize = sizeof(CHARFORMAT2W);
        TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cfw, 0);
        m_pTwh->SetCharFormat(cfw);
        return true;
    }
    return false;
}

DWORD CRichEditUI::GetSelectionCharFormat(CHARFORMAT2 &cf) const
{
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cf, &lResult);
    return (DWORD)lResult;
}

bool CRichEditUI::SetSelectionCharFormat(CHARFORMAT2 &cf)
{
    if( !m_pTwh ) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &lResult);
    return (BOOL)lResult == TRUE;
}

bool CRichEditUI::SetWordCharFormat(CHARFORMAT2 &cf)
{
    if( !m_pTwh ) return false;
    cf.cbSize = sizeof(CHARFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION|SCF_WORD, (LPARAM)&cf, &lResult); 
    return (BOOL)lResult == TRUE;
}

DWORD CRichEditUI::GetParaFormat(PARAFORMAT2 &pf) const
{
    pf.cbSize = sizeof(PARAFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_GETPARAFORMAT, 0, (LPARAM)&pf, &lResult);
    return (DWORD)lResult;
}

bool CRichEditUI::SetParaFormat(PARAFORMAT2 &pf)
{
    if( !m_pTwh ) return false;
    pf.cbSize = sizeof(PARAFORMAT2);
    LRESULT lResult;
    TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf, &lResult);
    if( (BOOL)lResult == TRUE ) {
        m_pTwh->SetParaFormat(pf);
        return true;
    }
    return false;
}

bool CRichEditUI::Redo()
{ 
    if( !m_pTwh ) return false;
    LRESULT lResult;
    TxSendMessage(EM_REDO, 0, 0, &lResult);
    return (BOOL)lResult == TRUE; 
}

bool CRichEditUI::Undo()
{ 
    if( !m_pTwh ) return false;
    LRESULT lResult;
    TxSendMessage(EM_UNDO, 0, 0, &lResult);
    return (BOOL)lResult == TRUE; 
}

void CRichEditUI::Clear()
{ 
    TxSendMessage(WM_CLEAR, 0, 0, 0); 
}

void CRichEditUI::Copy()
{ 
    TxSendMessage(WM_COPY, 0, 0, 0); 
}

void CRichEditUI::Cut()
{ 
    TxSendMessage(WM_CUT, 0, 0, 0); 
}

void CRichEditUI::Paste()
{ 
    TxSendMessage(WM_PASTE, 0, 0, 0); 
}

int CRichEditUI::GetLineCount() const
{ 
    if( !m_pTwh ) return 0;
    LRESULT lResult;
    TxSendMessage(EM_GETLINECOUNT, 0, 0, &lResult);
    return (int)lResult; 
}

CDuiString CRichEditUI::GetLine(int nIndex, int nMaxLength) const
{
    LPWSTR lpText = NULL;
    lpText = new WCHAR[nMaxLength + 1];
    ::ZeroMemory(lpText, (nMaxLength + 1) * sizeof(WCHAR));
    *(LPWORD)lpText = (WORD)nMaxLength;
    TxSendMessage(EM_GETLINE, nIndex, (LPARAM)lpText, 0);
    CDuiString sText;
    sText = (LPCWSTR)lpText;
    delete[] lpText;
    return sText;
}

int CRichEditUI::LineIndex(int nLine) const
{
    LRESULT lResult;
    TxSendMessage(EM_LINEINDEX, nLine, 0, &lResult);
    return (int)lResult;
}

int CRichEditUI::LineLength(int nLine) const
{
    LRESULT lResult;
    TxSendMessage(EM_LINELENGTH, nLine, 0, &lResult);
    return (int)lResult;
}

bool CRichEditUI::LineScroll(int nLines, int nChars)
{
    LRESULT lResult;
    TxSendMessage(EM_LINESCROLL, nChars, nLines, &lResult);
    return (BOOL)lResult == TRUE;
}

CPoint CRichEditUI::GetCharPos(long lChar) const
{ 
    CPoint pt; 
    TxSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)lChar, 0); 
    return pt;
}

long CRichEditUI::LineFromChar(long nIndex) const
{ 
    if( !m_pTwh ) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_EXLINEFROMCHAR, 0, nIndex, &lResult);
    return (long)lResult;
}

CPoint CRichEditUI::PosFromChar(UINT nChar) const
{ 
    POINTL pt; 
    TxSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, nChar, 0); 
    return CPoint(pt.x, pt.y); 
}

int CRichEditUI::CharFromPos(CPoint pt) const
{ 
    POINTL ptl = {pt.x, pt.y}; 
    if( !m_pTwh ) return 0;
    LRESULT lResult;
    TxSendMessage(EM_CHARFROMPOS, 0, (LPARAM)&ptl, &lResult);
    return (int)lResult; 
}

void CRichEditUI::EmptyUndoBuffer()
{ 
    TxSendMessage(EM_EMPTYUNDOBUFFER, 0, 0, 0); 
}

UINT CRichEditUI::SetUndoLimit(UINT nLimit)
{ 
    if( !m_pTwh ) return 0;
    LRESULT lResult;
    TxSendMessage(EM_SETUNDOLIMIT, (WPARAM) nLimit, 0, &lResult);
    return (UINT)lResult; 
}

long CRichEditUI::StreamIn(int nFormat, EDITSTREAM &es)
{ 
    if( !m_pTwh ) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_STREAMIN, nFormat, (LPARAM)&es, &lResult);
    return (long)lResult;
}

long CRichEditUI::StreamOut(int nFormat, EDITSTREAM &es)
{ 
    if( !m_pTwh ) return 0L;
    LRESULT lResult;
    TxSendMessage(EM_STREAMOUT, nFormat, (LPARAM)&es, &lResult);
    return (long)lResult; 
}
void CRichEditUI::FireShowOrHideScrollbar()
{
	IN_MSG msg;
	msg.nMsg = SCROLL_ShowOrHide;
	if( m_pHorizontalScrollBar )
	{
		msg.nBarType    = SB_HORZ;
		msg.bShowOrHide = m_pHorizontalScrollBar->IsVisible();
		OnScrollBar( (void*)&msg );
	}
	if( m_pVerticalScrollBar )
	{
		msg.nBarType = SB_VERT;
		msg.bShowOrHide = m_pVerticalScrollBar->IsVisible();
		OnScrollBar( (void*)&msg );
	}
}
void CRichEditUI::DoInit()
{
	if(m_bInited)
		return ;
    CREATESTRUCT cs;
    cs.style = m_lTwhStyle|ES_SELFIME;
    cs.x = 0;
    cs.y = 0;
    cs.cy = 0;
    cs.cx = 0;
    cs.lpszName = m_sText.GetData();
    CreateHost(this, &cs, &m_pTwh);
    if( m_pTwh )
	{
        m_pTwh->SetTransparent(TRUE);
        LRESULT lResult;
        m_pTwh->GetTextServices()->TxSendMessage(EM_SETLANGOPTIONS, 0, 0, &lResult);
        m_pTwh->OnTxInPlaceActivate(NULL);
		m_pTwh->UpdateInset();
		m_pManager->AddMessageFilter(this);
    }
	//
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	bool bIsWindowsXPLater = ((osvi.dwMajorVersion > 6)||( (osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 0) ));	
	if( cInitTextMax != GetLimitText() )
	{
		SetLimitText(m_iLimitText);
	}
	m_bInited= true;
}
bool CRichEditUI::IsPassword()
{
	return ( (m_lTwhStyle&ES_PASSWORD)==ES_PASSWORD );
}
HRESULT CRichEditUI::TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const
{
    if( m_pTwh ) {
        if( msg == WM_KEYDOWN && TCHAR(wparam) == VK_RETURN ) {
            if( !m_bWantReturn || (::GetKeyState(VK_CONTROL) < 0 && !m_bWantCtrlReturn) ) {
                if( m_pManager != NULL ) m_pManager->SendNotify((CControlUI*)this, DUI_MSGTYPE_RETURN);
                return S_OK;
            }
        }
        return m_pTwh->GetTextServices()->TxSendMessage(msg, wparam, lparam, plresult);
    }
    return S_FALSE;
}

IDropTarget* CRichEditUI::GetTxDropTarget()
{
    IDropTarget *pdt = NULL;
    if( m_pTwh->GetTextServices()->TxGetDropTarget(&pdt) == NOERROR ) return pdt;
    return NULL;
}

bool CRichEditUI::OnTxViewChanged()
{
	return m_bViewChanged;
}
SIZE CRichEditUI::GetScrollMove()
{
	SIZE szRet = { 0,0 };
	if( m_pTwh )
	{
		return m_pTwh->szScrollMove;
	}
	return szRet;
}
void CRichEditUI::EnableThrowScrollEvent(bool bThrow)
{
	if( m_pTwh )
		m_pTwh->bThrowEvent = bThrow;
}
bool CRichEditUI::EnableViewChanged(bool bChanged)
{
	bool bRet = m_bViewChanged;
	m_bViewChanged = bChanged;
	return m_bViewChanged;
}
bool CRichEditUI::SetDropAcceptFile(bool bAccept) 
{
	LRESULT lResult;
	DWORD dwMask = GetEventMask();
	dwMask |= ENM_DROPFILES|ENM_LINK;
	TxSendMessage(EM_SETEVENTMASK,0,dwMask, // ENM_CHANGE| ENM_CORRECTTEXT | ENM_DRAGDROPDONE | ENM_DROPFILES | ENM_IMECHANGE | ENM_LINK | ENM_OBJECTPOSITIONS | ENM_PROTECTED | ENM_REQUESTRESIZE | ENM_SCROLL | ENM_SELCHANGE | ENM_UPDATE,
		&lResult);
	return (BOOL)lResult == FALSE;
}

void CRichEditUI::OnTxNotify(DWORD iNotify, void *pv)
{
	switch(iNotify)
	{ 
	case EN_DROPFILES:   
	case EN_MSGFILTER:   
	case EN_OLEOPFAILED:   
	case EN_PROTECTED:   
	case EN_SAVECLIPBOARD:   
	case EN_SELCHANGE:   
	case EN_STOPNOUNDO:   
	case EN_LINK:   
	case EN_OBJECTPOSITIONS:   
	case EN_DRAGDROPDONE:   
	case EN_CHANGE:
		{
			if(pv)                        // Fill out NMHDR portion of pv   
			{   
				LONG nId =  GetWindowLong(this->GetManager()->GetPaintWindow(), GWL_ID);   
				NMHDR  *phdr = (NMHDR *)pv;   
				phdr->hwndFrom = this->GetManager()->GetPaintWindow();   
				phdr->idFrom = nId;   
				phdr->code = iNotify;  

				if(SendMessage(this->GetManager()->GetPaintWindow(), WM_NOTIFY, (WPARAM) nId, (LPARAM) pv))   
				{   
					//hr = S_FALSE;   
				}   
			}    
		}
		break;
	}
}

// 多行非rich格式的richedit有一个滚动条bug，在最后一行是空行时，LineDown和SetScrollPos无法滚动到最后
// 引入iPos就是为了修正这个bug
void CRichEditUI::SetScrollPos(SIZE szPos)
{
    int cx = 0;
    int cy = 0;
    if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
        m_pVerticalScrollBar->SetScrollPos(szPos.cy);
        cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
        int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
        m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
        cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
    }
    if( cy != 0 )
	{
        int iPos = 0;
        if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
            iPos = m_pVerticalScrollBar->GetScrollPos();
        if( m_pTwh )
		   m_pTwh->bThrowEvent = false;
		POINT ptOld={0,0};
		POINT ptCur={0,0};
		TxSendMessage(EM_GETSCROLLPOS,0,(LPARAM)(void*)&ptOld,0);

	    WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pVerticalScrollBar->GetScrollPos());
        TxSendMessage(WM_VSCROLL, wParam, 0L, 0);

		TxSendMessage(EM_GETSCROLLPOS,0,(LPARAM)(void*)&ptCur,0);

		if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
		{
            if( cy > 0 && m_pVerticalScrollBar->GetScrollPos() <= iPos )
                m_pVerticalScrollBar->SetScrollPos(iPos);
        }
		if( m_pTwh )
		{
			CRichEditUI::IN_MSG	param;
			param.nMsg = CRichEditUI::SCROLL_MovePos;
			param.nBarType = SB_VERT;
			param.nMovePos = ptCur.y - ptOld.y;
			
			IRichEditOle* pReOle = NULL;
			TxSendMessage( EM_GETOLEINTERFACE, 0, (LPARAM)&pReOle, 0);
			FireOleScroll(pReOle,0,param.nMovePos);
			if( pReOle )
				pReOle->Release();

			OnScrollBar( (void*)(&param) );
			//
			m_pTwh->bThrowEvent = true;
			Invalidate();
		}
    }
    if( cx != 0 ) {
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pHorizontalScrollBar->GetScrollPos());
        TxSendMessage(WM_HSCROLL, wParam, 0L, 0);
    }
}

void CRichEditUI::LineUp()
{
    TxSendMessage(WM_VSCROLL, SB_LINEUP, 0L, 0);
}

void CRichEditUI::LineDown()
{
    int iPos = 0;
    if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
        iPos = m_pVerticalScrollBar->GetScrollPos();
    TxSendMessage(WM_VSCROLL, SB_LINEDOWN, 0L, 0);
    if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
        if( m_pVerticalScrollBar->GetScrollPos() <= iPos )
            m_pVerticalScrollBar->SetScrollPos(m_pVerticalScrollBar->GetScrollRange());
    }
}

void CRichEditUI::PageUp()
{
	LineUp();
	//调用pageup微软的滚动位置计算错误
	//TxSendMessage(WM_VSCROLL, SB_PAGEUP, 0L, 0);
}

void CRichEditUI::PageDown()
{
	LineDown();
	//调用pagedown微软的滚动位置计算错误
	//TxSendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L, 0);
}

void CRichEditUI::HomeUp()
{
    TxSendMessage(WM_VSCROLL, SB_TOP, 0L, 0);
}

void CRichEditUI::EndDown()
{
    TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L, 0);
}

void CRichEditUI::LineLeft()
{
    TxSendMessage(WM_HSCROLL, SB_LINELEFT, 0L, 0);
}

void CRichEditUI::LineRight()
{
    TxSendMessage(WM_HSCROLL, SB_LINERIGHT, 0L, 0);
}

void CRichEditUI::PageLeft()
{
    TxSendMessage(WM_HSCROLL, SB_PAGELEFT, 0L, 0);
}

void CRichEditUI::PageRight()
{
    TxSendMessage(WM_HSCROLL, SB_PAGERIGHT, 0L, 0);
}

void CRichEditUI::HomeLeft()
{
    TxSendMessage(WM_HSCROLL, SB_LEFT, 0L, 0);
}

void CRichEditUI::EndRight()
{
    TxSendMessage(WM_HSCROLL, SB_RIGHT, 0L, 0);
}

void CRichEditUI::DoEvent(TEventUI& event)
{
    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }
	if( event.Type == UIEVENT_SCROLL_VISIBLE )
	{
		FireShowOrHideScrollbar();
	}
    if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
    {
        if( m_pTwh && m_pTwh->DoSetCursor(NULL, &event.ptMouse) ) {
            return;
        }
    }
    if( event.Type == UIEVENT_SETFOCUS ) {
        if( m_pTwh ) {
            m_pTwh->OnTxInPlaceActivate(NULL);
            m_pTwh->GetTextServices()->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
        }
		m_bFocused = true;
		Invalidate();
		return;
    }
    if( event.Type == UIEVENT_KILLFOCUS )  {
        if( m_pTwh ) {
            m_pTwh->OnTxInPlaceActivate(NULL);
            m_pTwh->GetTextServices()->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
        }
		m_bFocused = false;
		Invalidate();
		return;
    }
    if( event.Type == UIEVENT_TIMER ) {
        if( m_pTwh ) {
            m_pTwh->GetTextServices()->TxSendMessage(WM_TIMER, event.wParam, event.lParam, 0);
        } 
    }
    if( event.Type == UIEVENT_SCROLLWHEEL ) {
        if( (event.wKeyState & MK_CONTROL) != 0  ) {
            return;
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) 
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE ) 
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP ) 
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        return;
    }
    if( event.Type > UIEVENT__KEYBEGIN && event.Type < UIEVENT__KEYEND )
    {
        return;
    }
    CContainerUI::DoEvent(event);
}

SIZE CRichEditUI::EstimateSize(SIZE szAvailable)
{
    //return CSize(m_rcItem); // 这种方式在第一次设置大小之后就大小不变了
    return CContainerUI::EstimateSize(szAvailable);
}

void CRichEditUI::SetPos(RECT rc)
{
	RECT rcOld;	
	CControlUI::SetPos(rc);
    rc = m_rcItem;

    /* 微软已实现的逻辑，此处重复且存在计算错误
    rc.left += m_rcInset.left;
    rc.top += m_rcInset.top;
    rc.right -= m_rcInset.right;
    rc.bottom -= m_rcInset.bottom;*/
    bool bVScrollBarVisiable = false;
	
	if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
	{
        bVScrollBarVisiable = true;
        rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    }
    if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
	{
        rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
    }
    if( m_pTwh ) 
	{
		memcpy(&rcOld,m_pTwh->GetClientRect(),sizeof(RECT));
		m_pTwh->UpdateInset();
		m_pTwh->SetClientRect(&rc);
        if( bVScrollBarVisiable && (!m_pVerticalScrollBar->IsVisible() || m_bVScrollBarFixing) ) {
            LONG lWidth = rc.right - rc.left + m_pVerticalScrollBar->GetFixedWidth();
            LONG lHeight = 0;
            SIZEL szExtent = { -1, -1 };
            m_pTwh->GetTextServices()->TxGetNaturalSize(
                DVASPECT_CONTENT, 
                GetManager()->GetPaintDC(), 
                NULL,
                NULL,
                TXTNS_FITTOCONTENT,
                &szExtent,
                &lWidth,
                &lHeight);
            if( lHeight > rc.bottom - rc.top )
			{
                m_pVerticalScrollBar->SetVisible(true);
                m_pVerticalScrollBar->SetScrollPos(0);
                m_bVScrollBarFixing = true;
            
				FireEventUI(this,this->GetVerticalScrollBar(),UIEVENT_SCROLL_VISIBLE,
					this->GetVerticalScrollBar()->IsVisible());
			}
            else 
			{
                if( m_bVScrollBarFixing )
				{
                    m_pVerticalScrollBar->SetVisible(false);
                    m_bVScrollBarFixing = false;

					FireEventUI(this,this->GetVerticalScrollBar(),UIEVENT_SCROLL_VISIBLE,
						this->GetVerticalScrollBar()->IsVisible());
                }
            }
        }
    }
    if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) 
	{
        RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom};
        m_pVerticalScrollBar->SetPos(rcScrollBarPos);
    }
    if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) 
	{
        RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight()};
        m_pHorizontalScrollBar->SetPos(rcScrollBarPos);
    }
	if( m_pTwh && m_pTwh->_UiState == CTxtWinHost::ReDrawItem )
	{
		m_pTwh->_UiState = CTxtWinHost::ResetCmp;
		for( int it = 0; it < m_items.GetSize(); it++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) 
			{
				SetFloatPos(it);
			}
			else 
			{
				pControl->SetPos(rc); // 所有非float子控件放大到整个客户区
			}
		}
	}
	if( !::EqualRect(&rcOld,&rc) && bVScrollBarVisiable )
	{
		if( CTxtWinHost::SysEmpty==m_pTwh->_SysCmd )
		{
			//用户调整尺寸
			float nRange = m_pVerticalScrollBar->GetScrollRange();
			float nPos   = m_pVerticalScrollBar->GetScrollPos();			
			if(m_pVerticalScrollBar->GetScrollRange()==m_pVerticalScrollBar->GetScrollPos())
				m_pTwh->m_fScrollThum=1.0;
			else
				m_pTwh->m_fScrollThum = nPos/nRange;
		}
		else if( CTxtWinHost::SysRestore==m_pTwh->_SysCmd )
		{
			//还原时收到的第一次setpos是还原控件长宽
			m_pTwh->_SysCmd = CTxtWinHost::SysEmpty;
			if( m_pTwh->m_fScrollThum == 1.0 )
			{
				TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L, 0);
			}
			else
			{
				int nOldThum = m_pVerticalScrollBar->GetScrollRange()*m_pTwh->m_fScrollThum;
				WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION,nOldThum);
				TxSendMessage(WM_VSCROLL, wParam, 0L, 0);
			}
		}
	}
}
void CRichEditUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    RECT rcTemp = { 0 };
    if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return;

    CRenderClip clip;
    CRenderClip::GenerateClip(hDC, rcTemp, clip);
    CControlUI::DoPaint(hDC, rcPaint);

    if( m_items.GetSize() > 0 ) 
	{
        RECT rc = m_rcItem;
        /*rc.left += m_rcInset.left;
        rc.top += m_rcInset.top;
        rc.right -= m_rcInset.right;
        rc.bottom -= m_rcInset.bottom;*/
        if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

        if( !::IntersectRect(&rcTemp, &rcPaint, &rc) ) {
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
                if( !pControl->IsVisible() ) continue;
                if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl ->IsFloat() ) {
                    if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    pControl->DoPaint(hDC, rcPaint);
                }
            }
        }
        else {
            CRenderClip childClip;
            CRenderClip::GenerateClip(hDC, rcTemp, childClip);
            for( int it = 0; it < m_items.GetSize(); it++ ) {
                CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
                if( !pControl->IsVisible() ) continue;
                if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
                if( pControl ->IsFloat() ) {
                    if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
                    CRenderClip::UseOldClipBegin(hDC, childClip);
                    pControl->DoPaint(hDC, rcPaint);
                    CRenderClip::UseOldClipEnd(hDC, childClip);
                }
                else {
                    if( !::IntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
                    pControl->DoPaint(hDC, rcPaint);
                }
            }
        }
    }
	if( m_pTwh ) 
	{
		RECT rc;
		m_pTwh->GetControlRect(&rc);
		// Remember wparam is actually the hdc and lparam is the update
		// rect because this message has been preprocessed by the window.
		m_pTwh->GetTextServices()->TxDraw(
			DVASPECT_CONTENT,  		// Draw Aspect
			/*-1*/0,				// Lindex
			NULL,					// Info for drawing optimazation
			NULL,					// target device information
			hDC,			        // Draw device HDC
			NULL, 				   	// Target device HDC
			(RECTL*)&rc,			// Bounding client rectangle
			NULL, 		            // Clipping rectangle for metafiles
			(RECT*)&rcPaint,		// Update rectangle
			NULL, 	   				// Call back function
			NULL,					// Call back parameter
			0);				        // What view of the object
		if( m_bVScrollBarFixing ) {
			LONG lWidth = rc.right - rc.left + m_pVerticalScrollBar->GetFixedWidth();
			LONG lHeight = 0;
			SIZEL szExtent = { -1, -1 };
			m_pTwh->GetTextServices()->TxGetNaturalSize(
				DVASPECT_CONTENT, 
				GetManager()->GetPaintDC(), 
				NULL,
				NULL,
				TXTNS_FITTOCONTENT,
				&szExtent,
				&lWidth,
				&lHeight);
			if( lHeight <= rc.bottom - rc.top ) {
				NeedUpdate();
			}
		}
	}
    if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) 
	{
        if( ::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) 
		{
            m_pVerticalScrollBar->DoPaint(hDC, rcPaint);
        }
    }

    if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() )
	{
        if( ::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) 
		{
            m_pHorizontalScrollBar->DoPaint(hDC, rcPaint);
        }
    }
	if( m_pTwh )
	{
		m_pTwh->NotifyDrawItem();
	}
}

void CRichEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("vscrollbar")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_lTwhStyle |= ES_DISABLENOSCROLL | WS_VSCROLL;
    }
    if( _tcscmp(pstrName, _T("autovscroll")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_lTwhStyle |= ES_AUTOVSCROLL;
    }
    if( 0 == _tcscmp(pstrName,_T("maxchar")) )
	{
		m_iLimitText = _ttoi(pstrValue);
	}
	else if( _tcscmp(pstrName, _T("hscrollbar")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_lTwhStyle |= ES_DISABLENOSCROLL | WS_HSCROLL;
    }
    if( _tcscmp(pstrName, _T("autohscroll")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_lTwhStyle |= ES_AUTOHSCROLL;
    }
    else if( _tcscmp(pstrName, _T("wanttab")) == 0 ) {
        SetWantTab(_tcscmp(pstrValue, _T("true")) == 0);
    }
    else if( _tcscmp(pstrName, _T("wantreturn")) == 0 ) {
        SetWantReturn(_tcscmp(pstrValue, _T("true")) == 0);
    }
    else if( _tcscmp(pstrName, _T("wantctrlreturn")) == 0 ) {
        SetWantCtrlReturn(_tcscmp(pstrValue, _T("true")) == 0);
    }
    else if( _tcscmp(pstrName, _T("rich")) == 0 ) {
        SetRich(_tcscmp(pstrValue, _T("true")) == 0);
    }
    else if( _tcscmp(pstrName, _T("multiline")) == 0 ) {
        if( _tcscmp(pstrValue, _T("false")) == 0 ) m_lTwhStyle &= ~ES_MULTILINE;
    }
    else if( _tcscmp(pstrName, _T("readonly")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) { m_lTwhStyle |= ES_READONLY; m_bReadOnly = true; }
    }
    else if( _tcscmp(pstrName, _T("password")) == 0 ) {
        if( _tcscmp(pstrValue, _T("true")) == 0 ) m_lTwhStyle |= ES_PASSWORD;
    }
    else if( _tcscmp(pstrName, _T("align")) == 0 ) {
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_lTwhStyle &= ~(ES_CENTER | ES_RIGHT);
            m_lTwhStyle |= ES_LEFT;
        }
        if( _tcsstr(pstrValue, _T("center")) != NULL ) {
            m_lTwhStyle &= ~(ES_LEFT | ES_RIGHT);
            m_lTwhStyle |= ES_CENTER;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_lTwhStyle &= ~(ES_LEFT | ES_CENTER);
            m_lTwhStyle |= ES_RIGHT;
        }
    }
    else if( _tcscmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
    else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetTextColor(clrColor);
    }
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

LRESULT CRichEditUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if( !IsVisible() || !IsEnabled() ) return 0;
    //if( !IsMouseEnabled() && uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST ) return 0;
	if( !IsMouseEnabled() && uMsg > WM_MOUSEFIRST && uMsg <= WM_MOUSELAST ) return 0;
	if( uMsg == WM_MOUSEWHEEL && (LOWORD(wParam) & MK_CONTROL) == 0 ) 
		return 0;
	if( uMsg==WM_HELP )
	{
		bHandled = true;
		return 0;
	}
	if( uMsg==WM_SYSCOMMAND || uMsg==WM_SIZE )
	{
		if( wParam==SC_MINIMIZE || wParam==SC_MAXIMIZE || wParam==SIZE_MINIMIZED )
		{
			m_pTwh->_SysCmd = CTxtWinHost::SysMin;
			if(  m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			{
				//缓存第一次最小化rc用于判断时候还原
				float nRange = m_pVerticalScrollBar->GetScrollRange();
				float nPos   = m_pVerticalScrollBar->GetScrollPos();
				if( nPos==nRange )
					m_pTwh->m_fScrollThum = 1.0;
				else
					m_pTwh->m_fScrollThum = nPos/nRange;
			}
		}
		else if( (wParam==SC_RESTORE || wParam==SIZE_RESTORED) && m_pTwh->_SysCmd==CTxtWinHost::SysMin )
		{
			m_pTwh->_SysCmd = CTxtWinHost::SysRestore;
			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			{
				if( m_pTwh->m_fScrollThum == 1.0 )
				{
					TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L, 0);
				}
				else
				{
					int nOldThum = m_pVerticalScrollBar->GetScrollRange()*m_pTwh->m_fScrollThum;
					WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION,nOldThum);
					TxSendMessage(WM_VSCROLL, wParam, 0L, 0);
				}
			}
		}
	}

    bool bWasHandled = true;
    if( (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || uMsg == WM_SETCURSOR ) {
        if( !m_pTwh->IsCaptured() ) {
            switch (uMsg) {
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
                {
                    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                    CControlUI* pHover = GetManager()->FindControl(pt);
                    if(pHover != this) {
                        bWasHandled = false;
                        return 0;
                    }
                }
                break;
            }
        }
        // Mouse message only go when captured or inside rect
        DWORD dwHitResult = m_pTwh->IsCaptured() ? HITRESULT_HIT : HITRESULT_OUTSIDE;
        if( dwHitResult == HITRESULT_OUTSIDE ) {
            RECT rc;
            m_pTwh->GetControlRect(&rc);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            if( uMsg == WM_MOUSEWHEEL ) ::ScreenToClient(GetManager()->GetPaintWindow(), &pt);
            if( ::PtInRect(&rc, pt) && !GetManager()->IsCaptured() ) dwHitResult = HITRESULT_HIT;
        }
        if( dwHitResult != HITRESULT_HIT ) return 0;
        if( uMsg == WM_SETCURSOR ) bWasHandled = false;
        else if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK || uMsg == WM_RBUTTONDOWN ) {
            SetFocus();
        }
    }
#ifdef _UNICODE
    else if( uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST ) {
#else
    else if( (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) || uMsg == WM_CHAR || uMsg == WM_IME_CHAR ) {
#endif
        if( !IsFocused() ) return 0;
    }
    else if( uMsg == WM_CONTEXTMENU ) {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ::ScreenToClient(GetManager()->GetPaintWindow(), &pt);
        CControlUI* pHover = GetManager()->FindControl(pt);
        if(pHover != this) {
            bWasHandled = false;
            return 0;
        }
    }
    if(WM_CHAR == uMsg)
    {
#ifndef _UNICODE
		// check if we are waiting for 2 consecutive WM_CHAR messages
		if ( IsAccumulateDBCMode() )
		{
			if ( (GetKeyState(VK_KANA) & 0x1) )
			{
				// turn off accumulate mode
				SetAccumulateDBCMode ( false );
				m_chLeadByte = 0;
			}
			else
			{
				if ( !m_chLeadByte )
				{
					// This is the first WM_CHAR message, 
					// accumulate it if this is a LeadByte.  Otherwise, fall thru to
					// regular WM_CHAR processing.
					if ( IsDBCSLeadByte ( (WORD)wParam ) )
					{
						// save the Lead Byte and don't process this message
						m_chLeadByte = (WORD)wParam << 8 ;

						//TCHAR a = (WORD)wParam << 8 ;
						return 0;
					}
				}
				else
				{
					// This is the second WM_CHAR message,
					// combine the current byte with previous byte.
					// This DBC will be handled as WM_IME_CHAR.
					wParam |= m_chLeadByte;
					uMsg = WM_IME_CHAR;

					// setup to accumulate more WM_CHAR
					m_chLeadByte = 0; 
				}
			}
		}
#endif
    }
    LRESULT lResult = 0;
    HRESULT Hr = TxSendMessage(uMsg, wParam, lParam, &lResult);
    if( Hr == S_OK ) bHandled = bWasHandled;
    else if( (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) || uMsg == WM_CHAR || uMsg == WM_IME_CHAR )
        bHandled = bWasHandled;
    else if( uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST ) {
        if( m_pTwh->IsCaptured() ) bHandled = bWasHandled;
    }
    return lResult;
}
void CRichEditUI::SetAccumulateDBCMode( bool bDBCMode )
{
	m_fAccumulateDBC = bDBCMode;
}

bool CRichEditUI::IsAccumulateDBCMode()
{
	return m_fAccumulateDBC;
}
void CRichEditUI::RemoveAll()
{
	OnScrollBar.clear();
	OnChangeRc.clear();
	__super::RemoveAll();
}
void CRichEditUI::KillFocus()
{
	if (IsVisible())
	{
		SetInternVisible(true);
		if( m_pTwh )
		{
			m_pTwh->OnTxInPlaceDeactivate();
			m_pTwh->GetTextServices()->TxSendMessage(WM_KILLFOCUS,0,0,0);
			m_bFocused = false;
		}
	}
}

void CRichEditUI::SetFocus()
{
	if (IsVisible())
	{
		SetInternVisible(true);
		if( m_pTwh )
		{
			m_pTwh->OnTxInPlaceActivate(NULL);
			m_pTwh->GetTextServices()->TxSendMessage(WM_SETFOCUS,0,0,0);
			m_bFocused = true;
			GetManager()->SetFocus(this);
		}
	}
}

} // namespace DuiLib
