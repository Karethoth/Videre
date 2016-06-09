#include "gui_text.hh"

using namespace std;
using namespace gui;

void gui::render_unicode(
	string_unicode text,
	gui::GuiVec2 position,
	const gui::Window &window,
	std::string font,
	float scale )
{
	
}

string_unicode gui::get_line_overflow(
	string_unicode text,
	float line_width,
	std::string font )
{
	return {};
}


void GuiLabel::render() const
{

}


void GuiLabel::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );
}


void GuiTextArea::render() const
{
}


void GuiTextArea::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );
}
