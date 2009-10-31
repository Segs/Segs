#pragma once
#include "wxOgrePanel.h"
class GameView : public wxOgrePanel
{
public:
	GameView(wxWindow* parent,int win_id,wxPoint pos=wxDefaultPosition,wxSize sz = wxDefaultSize,int style = 0);
	virtual ~GameView(void){};
};
