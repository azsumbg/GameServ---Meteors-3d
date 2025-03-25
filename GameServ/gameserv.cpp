#include "pch.h"
#include "Gameserv.h"

// RANDIT ************************

dll::RANDIT::RANDIT()
{
	sq = new std::seed_seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
	twister = new std::mt19937(*sq);
}
dll::RANDIT::~RANDIT()
{
	if (sq)delete sq;
	if (twister)delete twister;
}

int dll::RANDIT::operator() (int min, int max)
{
	std::uniform_int_distribution<> distrib{ min, max };

	return distrib(*twister);
}

//////////////////////////////////

// PROTON ************************

dll::PROTON::PROTON(float _x, float _y, float _width, float _height) :start{ _x,_y }
{
	width = _width;
	height = _height;

	end.x = start.x + width;
	end.y = start.y + height;

	center.x = start.x + width / 2;
	center.y = start.y + height / 2;

	x_radius = width / 2;
	y_radius = height / 2;

	Rect.left = start.x;
	Rect.top = start.y;
	Rect.right = end.x;
	Rect.bottom = end.y;
}

float dll::PROTON::GetWidth() const
{
	return width;
}
float dll::PROTON::GetHeight() const
{
	return height;
}

void dll::PROTON::SetWidth(float _new_width)
{
	width = _new_width;
	end.x = start.x + width;
}
void dll::PROTON::SetHeight(float _new_height)
{
	height = _new_height;
	end.y = start.y + height;
}

void dll::PROTON::SetEdges()
{
	end.x = start.x + width;
	end.y = start.y + height;

	center.x = start.x + width / 2;
	center.y = start.y + height / 2;

	Rect.left = start.x;
	Rect.top = start.y;
	Rect.right = end.x;
	Rect.bottom = end.y;
}
void dll::PROTON::NewDims(float _new_width, float _new_height)
{
	width = _new_width;
	height = _new_height;

	end.x = start.x + width;
	end.y = start.y + height;

	center.x = start.x + width / 2;
	center.y = start.y + height / 2;

	x_radius = width / 2;
	y_radius = height / 2;

	Rect.left = start.x;
	Rect.top = start.y;
	Rect.right = end.x;
	Rect.bottom = end.y;
}

//////////////////////////////////

// BASE CLASS ********************

dll::BASE::BASE(uint8_t _what, float __x, float __y, float __to_x, float __to_y) :PROTON(__x, __y)
{
	type = _what;

	switch (type)
	{
	case type_small_star:
		speed = 1.0f;
		NewDims(8.0f, 10.0f);
		break;

	case type_mid_star:
		speed = 1.0f;
		NewDims(9.5f, 11.0f);
		break;

	case type_big_star:
		speed = 1.0f;
		NewDims(12.0f, 15.0f);
		break;

	case type_meteor1:
		NewDims(60.0f, 60.0f);
		speed = 0.8f;
		max_frames = 126;
		frame_delay = 1;
		lifes = 100;
		break;

	case type_meteor2:
		NewDims(50.0f, 116.0f);
		speed = 0.6f;
		max_frames = 49;
		frame_delay = 2;
		lifes = 120;
		break;

	case type_meteor3:
		NewDims(200.0f, 200.0f);
		speed = 0.4f;
		max_frames = 19;
		frame_delay = 4;
		lifes = 150;
		break;

	case type_left_laser:
		speed = 5.0f;
		NewDims(32.0f, 32.0f);
		SetPath(__to_x, __to_y);
		break;

	case type_right_laser:
		speed = 5.0f;
		NewDims(32.0f, 32.0f);
		SetPath(__to_x, __to_y);
		break;
	}
}
void dll::BASE::SetPath(float _dest_x, float _dest_y)
{
	hor_line = false;
	vert_line = false;

	move_sx = start.x;
	move_sy = start.y;

	move_ex = _dest_x;
	move_ey = _dest_y;

	if (move_sx == move_ex || (move_sx < move_ex && end.x >= move_ex) || (move_sx > move_ex && start.x - width <= move_ex))
	{
		vert_line = true;
		return;
	}
	if (move_sy == move_ey || (move_sy < move_ey && end.y >= move_ey) || (move_sy > move_ey && start.y - height <= move_ey))
	{
		hor_line = true;
		return;
	}

	slope = (move_ey - move_sy) / (move_ex - move_sx);
	intercept = move_sy - move_sx * slope;
}
int dll::BASE::GetFrame()
{
	--frame_delay;
	if (frame_delay <= 0)
	{
		switch (type)
		{
		
		case type_meteor1:
			frame_delay = 1;
			break;

		case type_meteor2:
			frame_delay = 2;
			break;

		case type_meteor3:
			frame_delay = 4;
			break;
		}

		++frame;
		if (frame < max_frames)frame = 0;
	}

	return frame;
}

////////////////////////////////// 

// STARS *************************

dll::STARS::STARS(uint8_t which, float _where_x, float _where_y) :BASE(which, _where_x, _where_y){}
bool dll::STARS::Move(float gear, bool canvas_move, dirs to_where)
{
	float now_speed = speed + gear / 10.0f;

	switch (dir)
	{
	case dirs::up:
		start.y -= now_speed;
		SetEdges();
		if (start.y >= sky) return true;
		break;

	case dirs::down:
		start.y += now_speed;
		SetEdges();
		if (end.y <= ground)return true;
		break;

	case dirs::left:
		start.x -= now_speed;
		SetEdges();
		if (start.x >= 0)return true;
		break;

	case dirs::right:
		start.x += now_speed;
		SetEdges();
		if (end.x <= scr_width)return true;
		break;

	case dirs::up_left:
		start.x -= now_speed;
		start.y -= now_speed;
		SetEdges();
		if (start.x >= 0 && start.y >= sky)return true;
		break;

	case dirs::up_right:
		start.x += now_speed;
		start.y -= now_speed;
		SetEdges();
		if (end.x <= scr_width && start.y >= sky)return true;
		break;

	case dirs::down_left:
		start.x -= now_speed;
		start.y += now_speed;
		SetEdges();
		if (start.x >= 0 && end.y <= ground)return true;
		break;

	case dirs::down_right:
		start.x += now_speed;
		start.y += now_speed;
		SetEdges();
		if (end.x <= scr_width && end.y <= ground)return true;
		break;
	}
	return false;
}
void dll::STARS::Release()
{
	delete this;
}

//////////////////////////////////

// METEORS **********************

dll::METEORS::METEORS(uint8_t which, float _where_x, float _where_y) :BASE(which, _where_x,
	_where_y) {};
bool dll::METEORS::Move(float gear, bool canvas_move, dirs to_where)
{
	float now_speed = speed + gear / 10.0f;

	start.x -= gear / 5;
	end.x += gear / 5;
	start.y -= gear / 5;
	end.y += gear / 5;

	center.x = start.x + width / 2;
	center.y = start.y + height / 2;

	x_radius = width / 2;
	y_radius = height / 2;

	Rect.left = start.x;
	Rect.top = start.y;
	Rect.right = end.x;
	Rect.bottom = end.y;

	if (start.x <= -scr_width / 3 || end.x >= scr_width + scr_width / 3
		|| start.y <= -scr_height / 3 || end.y >= scr_height + scr_height / 3)return false;

	if (canvas_move)
	{
		switch (to_where)
		{
		case dirs::left:
			SetPath(0, start.y);
			start.x -= now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;

		case dirs::right:
			SetPath(scr_width, start.y);
			start.x += now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;

		case dirs::up:
			SetPath(start.x, sky);
			if (start.x <= scr_width / 2)start.x += now_speed;
			else start.x -= now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;

		case dirs::down:
			SetPath(start.x, ground);
			if (start.x <= scr_width / 2)start.x += now_speed;
			else start.x -= now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;

		case dirs::up_left:
			SetPath(0, sky);
			start.x -= now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;

		case dirs::up_right:
			SetPath(scr_width, sky);
			start.x += now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;

		case dirs::down_left:
			SetPath(0, ground);
			start.x -= now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;

		case dirs::down_right:
			SetPath(scr_width, ground);
			start.x += now_speed;
			start.y = start.x * slope + intercept;
			SetEdges();
			break;
		}
	}

	return true;
}
void dll::METEORS::Release()
{
	delete this;
}

/////////////////////////////////

// LASERS ***********************

dll::LASERS::LASERS(uint8_t what_laser, float where_x, float where_y, float going_x, float going_y) :BASE(what_laser, where_x,
	where_y, going_x, going_y){}
bool dll::LASERS::Move(float gear, bool canvas_move, dirs to_where)
{
	float now_speed = speed + gear / 10.0f;

	if (type == type_left_laser)start.x += now_speed;
	else start.x -= now_speed;
	start.y = start.x * slope + intercept;
	SetEdges();
	if (end.x <= sky || start.y >= ground || end.x <= 0 || start.x >= scr_width)return false;

	return true;
}
void dll::LASERS::Release()
{
	delete this;
}

/////////////////////////////////

// FACTORY *********************

GAME_API dll::Object dll::Factory(uint8_t what, float on_x, float on_y, float to_x, float to_y)
{
	Object ret{ nullptr };

	switch (what)
	{
	case type_small_star:
		ret = new STARS(type_small_star, on_x, on_y);
		break;

	case type_mid_star:
		ret = new STARS(type_mid_star, on_x, on_y);
		break;

	case type_big_star:
		ret = new STARS(type_big_star, on_x, on_y);
		break;

	case type_meteor1:
		ret = new METEORS(type_meteor1, on_x, on_y);
		break;

	case type_meteor2:
		ret = new METEORS(type_meteor2, on_x, on_y);
		break;

	case type_meteor3:
		ret = new METEORS(type_meteor3, on_x, on_y);
		break;

	case type_left_laser:
		ret = new LASERS(type_left_laser, on_x, on_y, to_x, to_y);
		break;

	case type_right_laser:
		ret = new LASERS(type_right_laser, on_x, on_y, to_x, to_y);
		break;
	}

	return ret;
}