#pragma once

/* 声明事件映射宏 */	
#define DUI_BEGIN_NOTIFY(X)	virtual void Notify(DuiLib::TNotifyUI& msg){

/* 按控件，事件类型，映射 */
#define DUI_MESSAGE(n,e,f)	if( (msg.pSender)&&(0 == _tcsicmp(msg.pSender->GetName(),n)) &&	\
	(0 == _tcsicmp(msg.sType,e)) )	{				\
	f(msg);		return;	}
/* 按控件类型，事件类型，映射 */
#define DUI_CLASS_MSG(c,e,f) if((msg.pSender)&&(0 == _tcsicmp(msg.pSender->GetClass(),c)) &&	\
	(0 == _tcsicmp(msg.sType,e)) )	{				\
	f(msg);		return;	}
/* 按事件类型，映射 */
#define DUI_EVENT_MSG(e,f)   if( 0 == _tcsicmp(msg.sType,e) )	{ f(msg); return; }

/* 进行默认通知处理 */
#define DUI_DEFAULT(X)		 X::Notify(msg);

/* 结束事件映射宏 */
#define DUI_END_NOTIFY()		return; }