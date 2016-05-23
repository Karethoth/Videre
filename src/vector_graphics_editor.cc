#include "vector_graphics_editor.hh"
#include "gui.hh"
#include "gui_layouts.hh"
#include "gui_menu.hh"
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

	// Convert mouse drag end within the canvas to a mouse button released
	canvas->event_listeners.push_back(
	{
		GuiEventType::MOUSE_DRAG_END,
		[]( GuiElement *element, const GuiEvent &e )
		{
			if( element->in_area( e.mouse_drag_end.pos_start ) &&
				element->in_area( e.mouse_drag_end.pos_end ) )
			{
				GuiEvent event;
				event.type                = MOUSE_BUTTON;
				event.mouse_button.state  = RELEASED;
				event.mouse_button.pos    = e.mouse_drag_end.pos_end;
				event.mouse_button.button = e.mouse_drag_end.button;
				element->handle_event( event );
			}
		}
	} );

	// Add event listener for right mouse button within the canvas
	canvas->event_listeners.push_back(
	{
		GuiEventType::MOUSE_BUTTON,
		[&color_bg=color_bg]( GuiElement *element, const GuiEvent &e )
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
			popupMenu->color_bg = { 0.8, 0.8, 0.8, 0.6 };

			auto menu = make_shared<Menu>();

			auto set_to_act_as_button = []( GuiElement *element, glm::vec4 hover_color )
			{
				const auto normal_color = element->color_bg;

				element->event_listeners.push_back(
				{
					GuiEventType::MOUSE_ENTER,
					[hover_color]( GuiElement *element, const GuiEvent &e )
					{
						element->color_bg = hover_color;
					}
				} );

				element->event_listeners.push_back(
				{
					GuiEventType::MOUSE_LEAVE,
					[normal_color]( GuiElement *element, const GuiEvent &e )
					{
						element->color_bg = normal_color;
					}
				} );

				element->event_listeners.push_back(
				{
					GuiEventType::MOUSE_DRAG_END,
					[]( GuiElement *element, const GuiEvent &e )
					{
						if( element->in_area( e.mouse_drag_end.pos_start ) &&
						    element->in_area( e.mouse_drag_end.pos_end ) )
						{
							// If mouse drag starts and ends within this element
							// act as if it's just a mouse click
							GuiEvent event;
							event.type = MOUSE_BUTTON;
							event.mouse_button.state = RELEASED;
							event.mouse_button.pos = e.mouse_drag_end.pos_end;
							event.mouse_button.button = e.mouse_drag_end.button;
							element->handle_event( event );
						}
					}
				} );
			};

			// Add some test elements
			auto testMenuItem = make_shared<GuiElement>();
			testMenuItem->size = { 0, 25 };
			testMenuItem->color_bg = { 0.0, 0.0, 0.0, 0.8 };
			testMenuItem->event_listeners.push_back(
			{
				GuiEventType::MOUSE_BUTTON,
				[&color_bg = color_bg]( GuiElement *element, const GuiEvent &e )
				{
					if( e.mouse_button.button != 1 ||
					    e.mouse_button.state != RELEASED ||
					    !element->in_area( e.mouse_button.pos ) )
					{
						return;
					}

					color_bg = { 0.0, 0.0, 0.0, 0.0 };
				}
			} );
			set_to_act_as_button( testMenuItem.get(), { 0.0, 0.0, 0.0, 1.0 } );
			menu->add_child( testMenuItem );

			auto testMenuItem2 = make_shared<GuiElement>();
			testMenuItem2->size = { 0, 25 };
			testMenuItem2->color_bg = { 0.2, 0.3, 0.2, 0.8 };
			testMenuItem2->event_listeners.push_back(
			{
				GuiEventType::MOUSE_BUTTON,
				[&color_bg = color_bg]( GuiElement *element, const GuiEvent &e )
				{
					if( e.mouse_button.button != 1 ||
					    e.mouse_button.state != RELEASED ||
					    !element->in_area( e.mouse_button.pos ) )
					{
						return;
					}

					color_bg = { 0.2, 0.3, 0.2, 0.8 };
				}
			} );
			set_to_act_as_button( testMenuItem2.get(), { 0.2, 0.3, 0.2, 1.0 } );
			menu->add_child( testMenuItem2 );

			auto testMenuItem3 = make_shared<GuiElement>();
			testMenuItem3->size = { 0, 25 };
			testMenuItem3->color_bg = { 0.2, 0.2, 0.3, 0.8 };
			testMenuItem3->event_listeners.push_back(
			{
				GuiEventType::MOUSE_BUTTON,
				[&color_bg = color_bg]( GuiElement *element, const GuiEvent &e )
				{
					if( e.mouse_button.button != 1 ||
					    e.mouse_button.state != RELEASED ||
					    !element->in_area( e.mouse_button.pos ) )
					{
						return;
					}

					color_bg = { 0.2, 0.2, 0.3, 0.8 };
				}
			} );
			set_to_act_as_button( testMenuItem3.get(), { 0.2, 0.2, 0.3, 1.0 } );
			menu->add_child( testMenuItem3 );

			popupMenu->add_child( menu );

			// Resize the context menu
			GuiEvent event;
			event.type = MOVE;
			event.move.pos = e.mouse_button.pos;
			popupMenu->handle_event( event );

			event.type = RESIZE;
			event.resize.size = { 100, 0 };
			popupMenu->handle_event( event );

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

