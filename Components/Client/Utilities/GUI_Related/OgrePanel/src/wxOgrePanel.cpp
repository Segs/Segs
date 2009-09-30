#include "StdWx.h"
#include "wxOgrePanel.h"
using namespace Ogre;
class OgreInitializer
{
	static int m_users; 
	static OgreInitializer *m_instance;
	Ogre::Root *m_root;
	wxFrame *m_frm; // holds first context, needed for ogre's context handling quirkiness
	~OgreInitializer()
	{
	}
	void init_render_sys()
	{
		m_root = new Ogre::Root();
		m_root->setRenderSystem(m_root->getRenderSystemByName("OpenGL Rendering Subsystem"));
		m_root->initialise(false);
		m_frm = new wxFrame(0,-1,wxT(""));
		NameValuePairList a;
		a.insert(std::pair<String,String>("externalWindowHandle",StringConverter::toString( (size_t) m_frm->GetHandle() )));
		RenderSystem *sys = m_root->getRenderSystem();
		RenderWindow *m_ren = sys->_createRenderWindow(String("OgreRenderWindow_00"),1,1,false,&a);
		MaterialManager::getSingleton().initialise();
		m_frm->Show(false);
	}
	void close_render_sys()
	{
		if(0==m_frm)
			return;
		delete m_root;
		m_root=0;
		m_instance=0;
		m_frm->Close();
		m_frm=0;
	}
	OgreInitializer()
	{

	}
public:
	void attach(void *user)
	{
		if(m_users==0)
		{
			init_render_sys();
		}
		m_users++;
	}
	void detach(void *user)
	{
		if(--m_users==0)
			close_render_sys();
	}
	static OgreInitializer *instance()
	{
		if(!m_instance)
			m_instance = new OgreInitializer();
		return m_instance;
	}
	static Ogre::Root *root()
	{
		return instance()->m_root;
	}
};
OgreInitializer * OgreInitializer::m_instance=0;
int OgreInitializer::m_users=  0;
//IMPLEMENT_CLASS(wxOgrePanel, wxPanel)
BEGIN_EVENT_TABLE(wxOgrePanel, wxPanel)
	EVT_PAINT(wxOgrePanel::OnPaint) // Produces flickers and runs too fast!
	EVT_ERASE_BACKGROUND( wxOgrePanel::OnEraseBackground )
	EVT_SIZE(wxOgrePanel::OnSize)
	EVT_CLOSE(wxOgrePanel::OnClose)
/*
	EVT_TIMER( ID_RENDERTIMER, wxOgre::OnRenderTimer )
*/
END_EVENT_TABLE()
//IMPLEMENT_CLASS(wxOgrePanel, wxPanel)
wxOgrePanel::wxOgrePanel(wxWindow* parent,int win_id,wxPoint pos,wxSize sz,int style) : 
	wxPanel(parent,win_id,pos,sz,style)
{
	OgreInitializer::instance()->attach(this);
	SetMinSize(wxSize(1,1));
	createOgreRenderWindow();
}

wxOgrePanel::~wxOgrePanel(void)
{
//	RenderSystem *sys = g_root->getRenderSystem();
	OgreInitializer::root()->detachRenderTarget(m_RenderWindow);
	m_RenderWindow->destroy();
	OgreInitializer::instance()->detach(this);
//	sys->destroyRenderWindow(String("OgreRenderWindow")+m_unique_name);
}
void wxOgrePanel::update()
{
	m_RenderWindow->update();//renderOneFrame();
}
String wxOgrePanel::get_unique_name()
{
	return m_unique_name;
}
void wxOgrePanel::createOgreRenderWindow()
{
	int width;
	int height;
	RenderSystem *sys = OgreInitializer::root()->getRenderSystem();
	NameValuePairList a;

	a.insert(std::pair<String,String>("externalWindowHandle",StringConverter::toString( (size_t) this->GetHandle() )));
	GetSize(&width, &height);
	m_unique_name = StringConverter::toString((size_t) GetId());
	m_RenderWindow = sys->_createRenderWindow(String("OgreRenderWindow")+m_unique_name,width,height,false,&a);
	m_Root = OgreInitializer::root();
	createSceneManager();
	createCamera();
	m_ViewPort = m_RenderWindow->addViewport(m_Camera); 
	m_ViewPort->setBackgroundColour(Ogre::ColourValue(1.0f, ((GetId()*23)&0xFF)/255.0f, 0.0f, 1.0f));
	update();
}
void wxOgrePanel::OnClose(wxCloseEvent& event)
{
	event.Skip();
}
void wxOgrePanel::OnSize(wxSizeEvent& event)
{
	wxSize sz = event.GetSize();
	m_RenderWindow->resize( sz.x, sz.y );	
	m_RenderWindow->windowMovedOrResized();
	// Set the aspect ratio for the new size;
	if (m_Camera)
		m_Camera->setAspectRatio(Ogre::Real(sz.x) / Ogre::Real(sz.y));
	event.Skip();
}
void wxOgrePanel::OnPaint(wxPaintEvent& event)
{
	LogManager::getSingleton().logMessage(String("Pain for ")+m_unique_name+"\n");
	update();
	event.Skip();
}
void wxOgrePanel::OnEraseBackground( wxEraseEvent& )
{
}

void wxOgrePanel::createSceneManager()
{
	m_SceneMgr = m_Root->createSceneManager(Ogre::ST_GENERIC, String("ExampleSMInstance")+m_unique_name);
}

void wxOgrePanel::createCamera()
{
	m_Camera = m_SceneMgr->createCamera(String("PlayerCam")+m_unique_name);
	m_Camera->setPosition(Ogre::Vector3(0,0,500));
	m_Camera->lookAt(Ogre::Vector3(0,0,-300));
	m_Camera->setNearClipDistance(5);
}