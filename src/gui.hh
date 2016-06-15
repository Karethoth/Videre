#pragma once
#include "sdl2.hh"
#include <glm/glm.hpp>
#include <vector>
#include <functional>

namespace gui
{

struct GuiVec2;
struct GuiEvent;
struct GuiElement;
struct GuiMouseOverTracker;
struct GuiPixelOrPercentage;

enum GuiDirection    { UNDEFINED_DIRECTION, NORTH, SOUTH, EAST, WEST };
enum GuiDistanceType { PIXELS, PERCENTS, AUTO };
enum GuiButtonState  { RELEASED, PRESSED };
enum GuiEventType
{
	NO_EVENT, MOVE, RESIZE,
	MOUSE_BUTTON, MOUSE_SCROLL, MOUSE_MOVE, MOUSE_DRAG, MOUSE_DRAG_END, MOUSE_DOUBLE_CLICK,
	WINDOW_BLUR, WINDOW_FOCUS, MOUSE_ENTER, MOUSE_LEAVE
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
	glm::vec2 to_gl_vec() const { return { x, y }; };
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
			GuiVec2        pos;
			GuiButtonState state;
			int            button;
		} mouse_button;

		struct
		{
			GuiVec2      pos;
			GuiDirection direction;
			int          value;
		} mouse_scroll;

		struct
		{
			GuiVec2 pos;
		} mouse_move;

		struct
		{
			GuiVec2 pos_start;
			GuiVec2 pos_current;
			int     button;
		} mouse_drag;

		struct
		{
			GuiVec2 pos_start;
			GuiVec2 pos_end;
			int     button;
		} mouse_drag_end;

		struct
		{
			GuiVec2 pos;
			int     button;
		} mouse_double_click;
	};

	GuiEvent() {};
};



// Helper for generating events for MOUSE_ENTER and MOUSE_LEAVE
// from various mouse events
struct GuiMouseHoverHelper
{
	GuiMouseHoverHelper( const GuiElement &target );
	GuiEvent generate_event( const GuiEvent &e );

  protected:
	const GuiElement &target_element;
	bool is_over;
};



using GuiElementPtr = std::shared_ptr<GuiElement>;
using GuiEventListener = std::pair<
		decltype(GuiEvent::type),
		std::function<void(GuiElement*, const GuiEvent&)>>;



enum GuiElementStyleState
{
	NORMAL,
	HOVER
};



struct GuiElementStyle
{
	struct StyleRules
	{
		glm::vec4 color_bg   = { 0, 0, 0, 0 };
		glm::vec4 color_text = { 1, 1, 1, 1 };
		glm::vec4 padding    = { 0, 0, 0, 0 };
	};

	StyleRules normal;
	StyleRules hover;

	const StyleRules& get( const GuiElementStyleState state ) const;
};



struct GuiElement
{
	GuiVec2 pos;
	GuiVec2 size;

	GuiElement *parent = nullptr;
	std::vector<GuiElementPtr> children;

	GuiElementStyle style;
	GuiElementStyleState style_state;

	std::vector<GuiEventListener> event_listeners;

	GuiElement();
	virtual ~GuiElement() {};

	bool in_area( const GuiVec2 &_pos ) const;

	virtual void add_child( GuiElementPtr child );

	virtual GuiVec2 get_minimum_size() const;

	virtual void render() const;
	virtual void handle_event( const GuiEvent &e );
	virtual GuiElement *get_root() const;

  protected:
	virtual void init_child( GuiElement *child );
	GuiMouseHoverHelper hover_helper;
};

} // namespace gui

