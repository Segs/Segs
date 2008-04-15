#pragma once

#include <ace/ACE.h>
#include <ace/Get_Opt.h>
#include <ace/OS.h>
#include <ace/Singleton.h>
#include <ace/Task.h>
#include <wx/wx.h>

class wx_thr : public ACE_Task_Base
{
	HINSTANCE m_hInstance;
	HINSTANCE m_hPrevInstance;
	ACE_TCHAR *m_lpCmdLine;
	int m_nCmdShow;
public:
	void set_args(HINSTANCE hInstance, HINSTANCE hPrevInstance, ACE_TCHAR * lpCmdLine, int nCmdShow);
	virtual int svc (void);
};

class ExampleClient : public wxApp
{
public:
	bool OnInit();
	int OnExit();
};