#include "Avatar.h"
#include <wx/dc.h>
void StickFigures::HumanStickfigure::draw_on( wxDC *dc,int x,int y )
{
	int scale = 10;
	for(int i=0; i<5; i++)
	{
		Point start = m_points[m_sticks[i].beg];
		Point end = m_points[m_sticks[i].end];
		dc->DrawLine(x+start.m_x*scale,y+start.m_y*scale,x+end.m_x*scale,y+end.m_y*scale);
	}
}