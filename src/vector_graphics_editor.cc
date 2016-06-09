#include "vector_graphics_editor.hh"
#include "gui.hh"
#include "gui_text.hh"
#include "gui_menu.hh"
#include "gui_button.hh"
#include "gui_layouts.hh"
#include "gl_helpers.hh"
#include "globals.hh"
#include "text_helpers.hh"

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
	toolbar->style.normal.color_bg = { 0.0, 0.0, 0.0, 0.5 };
	toolbar->style.hover.color_bg = { 0.0, 0.0, 0.0, 0.5 };
	main_layout->add_child( toolbar );

	// Split the bottom into left and right parts
	auto sub_layout = make_shared<GridLayout>( 2, 1 );
	sub_layout->col_widths = { {0, AUTO}, {200, PIXELS} };
	main_layout->add_child( sub_layout );

	// Add canvas to the left element
	auto canvas = make_shared<VectorGraphicsCanvas>();
	canvas_element = canvas.get();
	canvas->style.normal.color_bg = { 1.0, 1.0, 1.0, 0.2 };
	canvas->style.hover.color_bg = { 1.0, 1.0, 1.0, 0.2 };
	sub_layout->add_child( canvas );

	// Add properties view to the right element
	auto properties = make_shared<VectorGraphicsProperties>();
	properties_element = properties.get();
	properties->style.normal.color_bg = { 0.0, 0.0, 0.0, 0.2 };
	properties->style.hover.color_bg = { 0.0, 0.0, 0.0, 0.2 };
	sub_layout->add_child( properties );

	add_child( main_layout );
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



VectorGraphicsCanvas::VectorGraphicsCanvas()
{
	// Set up the image
	image = vector_img::VectorImg();
	image.img_w = 320;
	image.img_h = 240;
	image.layers.emplace_back( new vector_img::ImgLayer() );

	camera_offset = { 0, 0 };
	scale = 1.f;
}



void VectorGraphicsCanvas::handle_event( const gui::GuiEvent &e )
{
	if( e.type == MOUSE_DRAG_END )
	{
		if( in_area( e.mouse_drag_end.pos_start ) &&
			in_area( e.mouse_drag_end.pos_end ) )
		{
			GuiEvent event;
			event.type                = MOUSE_BUTTON;
			event.mouse_button.state  = RELEASED;
			event.mouse_button.pos    = e.mouse_drag_end.pos_end;
			event.mouse_button.button = e.mouse_drag_end.button;
			handle_event( event );
		}
	}
	else if( e.type == MOUSE_BUTTON )
	{
		if( e.mouse_button.button != 3 ||
			e.mouse_button.state != RELEASED ||
			!in_area( e.mouse_button.pos ) )
		{
			auto window = dynamic_cast<Window*>(get_root());
			if( window )
			{
				window->popup_elements.clear();
			}

			return;
		}

		create_context_menu( e.mouse_button.pos );
	}
	else
	{
		GuiElement::handle_event( e );
	}
}


void VectorGraphicsCanvas::create_context_menu( GuiVec2 tgt_pos )
{
	auto window = dynamic_cast<Window*>(get_root());
	if( !window )
	{
		return;
	}

	window->popup_elements.clear();

	auto popup_menu = make_shared<PopupElement>();
	popup_menu->style.normal.color_bg = { 0.8, 0.8, 0.8, 0.6 };
	popup_menu->style.hover.color_bg = { 0.8, 0.8, 0.8, 0.6 };
	popup_menu->target_pos = tgt_pos;

	auto menu = make_shared<Menu>();

	// Add some test buttons
	auto button1 = make_shared<GuiButton>();
	button1->size = { 0, 25 };
	button1->style.normal.color_bg = { 0.0, 0.0, 0.0, 0.8 };
	button1->style.hover.color_bg  = { 0.0, 0.0, 0.0, 1.0 };
	button1->on_click = [&image=image, popup_menu, this]( GuiElement *element, const GuiEvent &e )
	{
		if( e.mouse_button.button != 1 ||
			e.mouse_button.state != RELEASED ||
			!element->in_area( e.mouse_button.pos ) )
		{
			return;
		}

		auto item = unique_ptr<vector_img::ImgControlPoint>( new vector_img::ImgControlPoint() );
		item->x = static_cast<float>( popup_menu->target_pos.x - this->pos.x);
		item->y = static_cast<float>( popup_menu->target_pos.y - this->pos.y );
		image.layers[0]->items.push_back( move( item ) );

		popup_menu->deleted = true;
	};
	
	auto button1_label = make_shared<GuiLabel>();
	button1_label->content = u8_to_unicode( "Test" );
	button1->add_child( button1_label );

	menu->add_child( button1 );

	// Add spacer
	auto spacer = make_shared<MenuSpacer>();
	spacer->style.normal.color_bg = glm::vec4{ 0, 0, 0, 0.6 };
	spacer->style.hover.color_bg = glm::vec4{ 0, 0, 0, 0.6 };
	spacer->size.h = 1;
	menu->add_child( spacer );

	// Add another button
	auto button2 = make_shared<GuiButton>();
	button2->size = { 0, 25 };
	button2->style.normal.color_bg = { 0.0, 0.0, 0.0, 0.8 };
	button2->style.hover.color_bg  = { 0.0, 0.0, 0.0, 1.0 };
	button2->on_click = [&image=image, popup_menu, this]( GuiElement *element, const GuiEvent &e )
	{
		if( e.mouse_button.button != 1 ||
			e.mouse_button.state != RELEASED ||
			!element->in_area( e.mouse_button.pos ) )
		{
			return;
		}

		auto item = unique_ptr<vector_img::ImgLine>( new vector_img::ImgLine() );
		item->a.x = static_cast<float>( popup_menu->target_pos.x - this->pos.x );
		item->a.y = static_cast<float>( popup_menu->target_pos.y - this->pos.y );
		item->b.x = static_cast<float>( popup_menu->target_pos.x - this->pos.x + 50 );
		item->b.y = static_cast<float>( popup_menu->target_pos.y - this->pos.y );
		image.layers[0]->items.push_back( move( item ) );

		popup_menu->deleted = true;
	};
	menu->add_child( button2 );

	popup_menu->add_child( menu );

	// Resize the context menu
	GuiEvent event;
	event.type = MOVE;
	event.move.pos = tgt_pos;
	popup_menu->handle_event( event );

	event.type = RESIZE;
	event.resize.size = { 100, 0 };
	popup_menu->handle_event( event );

	popup_menu->parent = window;


	// Check if the element would fit better above or on the left side of the target position
	event.type = MOVE;
	event.move.pos = tgt_pos;

	const auto context_menu_size = popup_menu->get_minimum_size();
	if( tgt_pos.x + context_menu_size.w > pos.x + size.w &&
		tgt_pos.x - context_menu_size.w >= pos.x )
	{
		event.move.pos.x = pos.x + size.w - context_menu_size.w;
	}

	if( tgt_pos.y + context_menu_size.h > pos.y + size.h &&
		tgt_pos.y - context_menu_size.h >= pos.y )
	{
		event.move.pos.y = pos.y + size.h - context_menu_size.h;
	}
	if( event.move.pos.x || event.move.pos.y )
	{
		popup_menu->handle_event( event );
	}

	window->popup_elements.push_back( popup_menu );
	
	// Send the current mouse position to the popup menu
	event.type = MOUSE_MOVE;
	SDL_GetMouseState( &event.mouse_move.pos.x, &event.mouse_move.pos.y );
	popup_menu->handle_event( event );
}



void VectorGraphicsCanvas::render() const
{
	render_vector_img();
}



void VectorGraphicsCanvas::render_vector_img() const
{
	auto shader = Globals::shaders.find( "2d" );

	const glm::vec2 center = {
		pos.x + size.w / 2.f + camera_offset.x,
		pos.y + size.h / 2.f + camera_offset.y
	};

	// "Soft" image size and position
	const glm::vec2 img_size = {
		image.img_w * scale,
		image.img_h * scale
	};

	const glm::vec2 img_pos = {
		center.x - img_size.x / 2.f,
		center.y - img_size.y / 2.f
	};

	// Visible area of the image
	glm::vec2 img_area_pos  = img_pos;
	glm::vec2 img_area_size = img_size;

	if( img_pos.x < pos.x )
	{
		img_area_pos.x = static_cast<float>( pos.x );
	}

	if( img_pos.y < pos.y  )
	{
		img_area_pos.y = static_cast<float>( pos.y );
	}

	const auto overflow_x = img_pos.x + img_size.x - pos.x  - size.w;
	const auto overflow_y = img_pos.y + img_size.y - pos.y  - size.h;

	if( overflow_x > 0 )
	{
		img_area_size.x -= overflow_x * 2;
	}

	if( overflow_y > 0 )
	{
		img_area_size.y -= overflow_y * 2;
	}

	glUseProgram( shader->second.program );

	const auto colorUniform = shader->second.get_uniform( "color" );
	glUniform4f( colorUniform, 1.f, 1.f, 1.f, 0.5f );

	const auto texturedUniform = shader->second.get_uniform( "textured" );
	glUniform1i( texturedUniform, 0 );

	gl::render_quad_2d(shader->second, get_root()->size.to_gl_vec(), img_area_pos, img_area_size);
}
