#include "Example01.h"
#include "LoginDialog.h"
#include <ace/Reactor.h>
#include <ogre/OgreException.h>
wx_thr thr; // creating instance of wx_thr, maybe it should be a singleton ? 
            // Remember: this thread is started from ACE_Main::run_i()

void wx_thr::set_args(HINSTANCE hInstance, HINSTANCE hPrevInstance, ACE_TCHAR * lpCmdLine, int nCmdShow)
{
    m_hInstance=hInstance;
    m_hPrevInstance=hPrevInstance;
    m_lpCmdLine=lpCmdLine;
    m_nCmdShow=nCmdShow;
}
int wx_thr::svc (void)
{
    return wxEntry(m_hInstance,m_hPrevInstance,m_lpCmdLine,m_nCmdShow);
}

IMPLEMENT_APP_NO_MAIN(ExampleClient) // important! main is created by ACE

bool ExampleClient::OnInit()
{
    wxInitAllImageHandlers();
    LoginDialog* login_test = new LoginDialog(NULL, wxID_ANY, wxEmptyString);
    SetTopWindow(login_test);
    login_test->ShowModal();
    OnExit();
    return false;
}
int ExampleClient::OnExit()
{
    ACE_Reactor::instance ()->end_reactor_event_loop();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// ACE main 
//////////////////////////////////////////////////////////////////////////
class ACE_Main : public ACE_Main_Base 
{
    int run_i (int argc, ACE_TCHAR *argv[]);
};

int ACE_Main::run_i (int argc, ACE_TCHAR *argv[])
{
    thr.activate(); // starts wxwidgets thread
    return ACE_Reactor::instance ()->run_reactor_event_loop ();
}
#ifdef _WIN32
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, ACE_TCHAR *lpCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{ 
#ifdef _WIN32
    int argc=0;
    char *argv[]={0};
#else
    //        thr.set_args(hInstance, hPrevInstance, lpCmdLine,nCmdShow); this needs implementation for *nix'es 
#endif
    try{
        ACE_Main m;
        char *t=0;
#ifdef _WIN32
        thr.set_args(hInstance, hPrevInstance, lpCmdLine,nCmdShow);
#else
//        thr.set_args(hInstance, hPrevInstance, lpCmdLine,nCmdShow); this needs implementation for *nix'es 
#endif
        m.run(argc,argv);
        thr.wait();
    } 
    catch(Ogre::Exception& e) // pseudo error handler
    {
        wxMessageBox(wxString::Format(wxT("OGRE error ! %s"),e.getDescription().c_str()),wxT("Warning"),wxICON_ERROR|wxOK);
    }
    return 0;
}