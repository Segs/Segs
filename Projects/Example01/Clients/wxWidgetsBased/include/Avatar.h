#pragma once
//#include "types.h"
class wxDC;
namespace StickFigures
{
	class Point
	{
	public:
		Point(double x=0,double y=0) : m_x(x),m_y(y) {}
		double m_x,m_y;
	};
	class Stick
	{
	public:
		Stick(int b=0,int e=0) : beg(b),end(e){}
		int beg,end;
	};
	class StickFigure
	{
	public:
		Point *m_points;
		Stick *m_sticks;
	};
	class HumanStickfigure : public StickFigure
	{
	public:
		void draw_on(wxDC *,int x,int y );
		HumanStickfigure()
		{
			m_points=new Point[7];
			int idx=0;
			m_points[idx++]=Point(0,0); // torso
			m_points[idx++]=Point(0,2);
			m_points[idx++]=Point(1,0); // left hand
			m_points[idx++]=Point(-1,0); // right hand
			m_points[idx++]=Point(1,4); // left leg
			m_points[idx++]=Point(-1,4); // right leg
			m_sticks=new Stick[5];
			idx=0;
			m_sticks[idx++] = Stick(0,1);
			m_sticks[idx++] = Stick(0,2);
			m_sticks[idx++] = Stick(0,3);
			m_sticks[idx++] = Stick(1,4);
			m_sticks[idx++] = Stick(1,5);
		}
	};
}
