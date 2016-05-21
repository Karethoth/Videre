#include "vector_graphics_editor.hh"
#include "gui_layouts.hh"
#include <memory>
#include <iostream>

using namespace std;
using namespace gui;


VectorGraphicsEditor::VectorGraphicsEditor()
{
	// Split the element to top and bottom halfs
	auto main_layout = make_shared<GridLayout>( 1, 2 );
	main_layout->row_heights = { {30, PIXELS}, {0, AUTO} };

	// Add toolbar to the top element
	auto toolbar = make_shared<VectorGraphicsToolbar>();
	toolbar->color_bg = { 0.0, 0.0, 0.0, 0.5 };
	main_layout->add_child( toolbar );

	// Split the bottom into left and right parts
	auto sub_layout = make_shared<GridLayout>( 2, 1 );
	sub_layout->col_widths = { {0, AUTO}, {200, PIXELS} };
	main_layout->add_child( sub_layout );

	// Add canvas to the left element
	auto canvas = make_shared<VectorGraphicsCanvas>();
	canvas_element = canvas.get();
	canvas->color_bg = { 1.0, 1.0, 1.0, 0.2 };
	sub_layout->add_child( canvas );

	// Add properties view to the right element
	auto properties = make_shared<VectorGraphicsProperties>();
	properties_element = properties.get();
	properties->color_bg = { 0.0, 0.0, 0.0, 0.2 };
	sub_layout->add_child( properties );

	add_child( main_layout );


	/*
	   Sketching out how context menus should work
	   - should be made easy to add to elements without too much bloat
	 */

	// Add event listener for right mouse button within the canvas
	canvas->event_listeners.push_back(
	{
		GuiEventType::MOUSE_BUTTON,
		[]( GuiElement *element, const GuiEvent &e )
		{
			auto root = element->get_root();
			auto window = dynamic_cast<Window*>( root );
			if( !window )
			{
				return;
			}

			if( e.mouse_button.button != 3 ||
			    e.mouse_button.state != RELEASED ||
			    !element->in_area( e.mouse_button.pos ) )
			{
				window->popup_elements.clear();
				return;
			}

			auto popups_exist = window->popup_elements.size() > 0;
			if( popups_exist )
			{
				window->popup_elements.clear();
			}

			auto popupMenu = make_shared<PopupElement>();
			popupMenu->color_bg = { 1.0, 0.0, 0.0, 1.0 };
			popupMenu->size = { 100, 100 };
			popupMenu->pos = e.mouse_button.pos;
			popupMenu->parent = window;
			// TODO: check if the element would fit better above the pointer,
			//       or on the left side

			window->popup_elements.push_back( popupMenu );
		}
	} );
}


VectorGraphicsEditor::~VectorGraphicsEditor()
{
}


void VectorGraphicsEditor::render() const
{
	GuiElement::render();
}


void VectorGraphicsEditor::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );
}

