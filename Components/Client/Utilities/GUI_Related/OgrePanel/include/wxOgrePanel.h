#pragma once
#include <wx/control.h>
#include <wx/panel.h>
#include <wx/timer.h>
#include <Ogre.h>

class wxOgrePanel : public wxPanel
{
	Ogre::String m_unique_name;
public:
//	DECLARE_CLASS(wxOgrePanel)
	wxOgrePanel(wxWindow* parent,int win_id,wxPoint pos=wxDefaultPosition,wxSize sz = wxDefaultSize,int style = 0);
	virtual ~wxOgrePanel(void);
	void update(); //! Renders a single Ogre frame
	inline Ogre::Camera* getCamera(){return m_Camera;}//! Returns the currently used camera
	inline void setCamera(Ogre::Camera* camera){m_Camera = camera;}//! Sets a new camera for rendering
	Ogre::String get_unique_name();

	Ogre::Root* getRoot() const { return m_Root; }
	Ogre::Viewport* getViewPort() const { return m_ViewPort; }
	Ogre::SceneManager* getSceneMgr() const { return m_SceneMgr; }
	Ogre::Camera* getCamera() const { return m_Camera; }
protected:
	DECLARE_EVENT_TABLE()
private:
	virtual void createSceneManager();
	virtual void createCamera();
	void initializeOgreRoot();
	void createOgreRenderWindow();
	void toggleTimerRendering();

	void OnClose(wxCloseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground( wxEraseEvent& );
	void OnRenderTimer(wxTimerEvent& event);
	wxTimer	mTimer;
protected:
	Ogre::Root* m_Root;
	Ogre::Viewport* m_ViewPort;
	Ogre::Camera* m_Camera;
	Ogre::SceneManager* m_SceneMgr;
	Ogre::RenderWindow* m_RenderWindow;
};
