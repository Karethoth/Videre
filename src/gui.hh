#pragma once
#include "sdl2.hh"
#include <vector>

namespace gui
{

enum GuiDirection    { UNDEFINED_DIRECTION, NORTH, SOUTH, EAST, WEST };
enum GuiDistanceType { PIXELS, PERCENTS, AUTO };
enum GuiButtonState  { RELEASED, PRESSED };
enum GuiEventType
{
	MOVE, RESIZE,
	MOUSE_BUTTON, MOUSE_SCROLL, MOUSE_MOVE, MOUSE_DRAG, MOUSE_DOUBLE_CLICK,
	WINDOW_BLUR, WINDOW_FOCUS
};



struct GuiVec2
{
	union
	{
		struct
		{
			int x;
			int y;
		};

		struct
		{
			int w;
			int h;
		};
	};

	GuiVec2( int _x=0, int _y=0 ) : x( _x ), y( _y ) {};
};


GuiVec2 operator+( const GuiVec2 &a, const GuiVec2 &b );
GuiVec2 operator-( const GuiVec2 &a, const GuiVec2 &b );



struct GuiPixelsOrPercentage
{
	GuiDistanceType type;
	int val;

	GuiPixelsOrPercentage(
		int _val = 0,
		const GuiDistanceType _type=PIXELS
	) : val(_val), type(_type) {}
};



struct GuiEvent
{
	GuiEventType type;

	union
	{
		struct
		{
			GuiVec2 pos;
		} move;

		struct
		{
			GuiVec2 size;
		} resize;

		struct
		{
			int button;
			GuiVec2 pos;
			GuiButtonState state;
		} mouse_button;

		struct
		{
			GuiDirection direction;
			int value;
		} mouse_scroll;

		struct
		{
			GuiVec2 pos;
		} mouse_move;

		struct
		{
			GuiVec2 pos_start;
			GuiVec2 pos_current;
		} mouse_drag;

		struct
		{
			int button;
			GuiVec2 pos;
		} mouse_double_click;
	};

	GuiEvent() {};
};

struct GuiElement;
typedef std::shared_ptr<GuiElement> GuiElementPtr;

struct GuiElement
{
	GuiVec2 pos;
	GuiVec2 size;

	GuiElement *parent = nullptr;
	std::vector<GuiElementPtr> children;


	GuiElement() {};
	virtual ~GuiElement() {};

	bool in_area( const GuiVec2 &_pos ) const;

	virtual void add_child( GuiElementPtr child );

	virtual GuiVec2 get_minimum_size() const;

	virtual void render() const;
	virtual void handle_event( const GuiEvent &e );

  protected:
	virtual void init_child( GuiElement *child );
};

} // namespace gui

