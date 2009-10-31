#include "GameView.h"
#include "Avatar.h"

using namespace Ogre;
//BEGIN_EVENT_TABLE(GameView,wxOgrePanel)

//END_EVENT_TABLE()
GameView::GameView( wxWindow* parent,int win_id,wxPoint pos/*=wxDefaultPosition*/,wxSize sz /*= wxDefaultSize*/,int style /*= 0*/ ) : 
		wxOgrePanel(parent,win_id,pos,sz,style)
{
	getViewPort()->setBackgroundColour(ColourValue(0.2f,0.4f,0.2f));
	getSceneMgr()->setAmbientLight(ColourValue(0.7f,0.7f,0.7f));
	getCamera()->setPosition(Vector3(0, 60, 60));
	getCamera()->lookAt(Vector3(0, 0, 0));
	getCamera()->setNearClipDistance(1);

}
