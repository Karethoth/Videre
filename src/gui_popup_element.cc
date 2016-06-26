#include "gui_popup_element.hh"
#include "window.hh"

using namespace std;
using namespace gui;


PopupElement::~PopupElement()
{
}



void PopupElement::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );

	if( e.type == RESIZE )
	{
		auto min_size = get_minimum_size();
		size.w = min_size.w;
		size.h = min_size.h;
	}
}



void PopupElement::render() const
{
	GuiElement::render();
}



PopupElementHandleHelper::PopupElementHandleHelper( GuiElement& parent )
: parent( parent ),
  handles()
{
}



void PopupElementHandleHelper::clear_popups()
{
	auto window = dynamic_cast<Window*>( parent.get_root() );
	if( !window )
	{
		return;
	}

	for( auto popup_handle : handles )
	{
		window->remove_popup( popup_handle );
	}

	handles.clear();
}

