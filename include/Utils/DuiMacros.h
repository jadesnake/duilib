#pragma once

/* �����¼�ӳ��� */	
#define DUI_BEGIN_NOTIFY(X)	virtual void Notify(DuiLib::TNotifyUI& msg){

/* ���ؼ����¼����ͣ�ӳ�� */
#define DUI_MESSAGE(n,e,f)	if( (msg.pSender)&&(0 == _tcsicmp(msg.pSender->GetName(),n)) &&	\
	(0 == _tcsicmp(msg.sType,e)) )	{				\
	f(msg);		return;	}
/* ���ؼ����ͣ��¼����ͣ�ӳ�� */
#define DUI_CLASS_MSG(c,e,f) if((msg.pSender)&&(0 == _tcsicmp(msg.pSender->GetClass(),c)) &&	\
	(0 == _tcsicmp(msg.sType,e)) )	{				\
	f(msg);		return;	}
/* ���¼����ͣ�ӳ�� */
#define DUI_EVENT_MSG(e,f)   if( 0 == _tcsicmp(msg.sType,e) )	{ f(msg); return; }

/* ����Ĭ��֪ͨ���� */
#define DUI_DEFAULT(X)		 X::Notify(msg);

/* �����¼�ӳ��� */
#define DUI_END_NOTIFY()		return; }