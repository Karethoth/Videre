#include "vector_graphics_editor.hh"
#include "gui.hh"
#include "gui_text.hh"
#include "gui_menu.hh"
#include "gui_button.hh"
#include "gui_layouts.hh"
#include "gl_helpers.hh"
#include "globals.hh"
#include "settings.hh"
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
	// TODO: Decide if context menu should or should not close
	//       when mouse is used in an another element
	const auto close_when_moused_elsewhere = true;

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
		else if( close_when_moused_elsewhere )
		{
			auto window = dynamic_cast<Window*>(get_root());
			if( window )
			{
				window->popup_elements.clear();
			}
		}
	}
	else if( e.type == MOUSE_BUTTON )
	{
		const auto is_in_area = in_area( e.mouse_button.pos );
		
		if( (close_when_moused_elsewhere || is_in_area) &&
		    (e.mouse_button.button != 3 || e.mouse_button.state != RELEASED) )
		{
			auto window = dynamic_cast<Window*>(get_root());
			if( window )
			{
				window->popup_elements.clear();
			}

			return;
		}
		else if( is_in_area )
		{
			create_context_menu( e.mouse_button.pos );
		}
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

	// Create a test button
	const auto label_padding = glm::vec4( 6.f, 4.f, 8.f, 8.f );

	auto button1 = make_shared<GuiButton>();
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

		const auto canvas_area = get_canvas_area();

		auto item = unique_ptr<vector_img::ImgControlPoint>( new vector_img::ImgControlPoint() );
		item->x = static_cast<float>( popup_menu->target_pos.x - canvas_area.x);
		item->y = static_cast<float>( popup_menu->target_pos.y - canvas_area.y );
		image.layers[0]->items.push_back( move( item ) );

		popup_menu->deleted = true;
	};
	
	auto button1_label = make_shared<GuiLabel>(
		u8_to_unicode( "\xE7\x8C\xAB\xE3\x81\xAF\xE5\xB0\x8F\xE3\x81\x95\xE3\x81\x84" )
	);
	button1_label->dynamic_font_size = true;
	button1_label->style.normal.color_text = glm::vec4{ 0.9f };
	button1_label->style.hover.color_text  = glm::vec4{ 1.0f };
	button1_label->style.normal.padding = label_padding;
	button1_label->style.hover.padding  = label_padding;
	button1->add_child( button1_label );

	menu->add_child( button1 );

	// Add spacer
	auto spacer = make_shared<MenuSpacer>();
	spacer->style.normal.color_bg = glm::vec4{ 0, 0, 0, 0.6 };
	spacer->style.hover.color_bg = glm::vec4{ 0, 0, 0, 0.6 };
	spacer->size.h = 1;
	menu->add_child( spacer );

	// Add another test button
	auto button2 = make_shared<GuiButton>();
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

		const auto canvas_area = get_canvas_area();

		auto item = unique_ptr<vector_img::ImgLine>( new vector_img::ImgLine() );
		item->a.x = static_cast<float>( popup_menu->target_pos.x - canvas_area.x - 2 );
		item->a.y = static_cast<float>( popup_menu->target_pos.y - canvas_area.y );
		item->b.x = static_cast<float>( popup_menu->target_pos.x - canvas_area.x + 50 );
		item->b.y = static_cast<float>( popup_menu->target_pos.y - canvas_area.y );
		image.layers[0]->items.push_back( move( item ) );

		auto item2 = unique_ptr<vector_img::ImgLine>( new vector_img::ImgLine() );
		item2->a.x = static_cast<float>( popup_menu->target_pos.x - canvas_area.x );
		item2->a.y = static_cast<float>( popup_menu->target_pos.y - canvas_area.y - 2 );
		item2->b.x = static_cast<float>( popup_menu->target_pos.x - canvas_area.x );
		item2->b.y = static_cast<float>( popup_menu->target_pos.y - canvas_area.y + 50 );
		image.layers[0]->items.push_back( move( item2 ) );

		popup_menu->deleted = true;
	};
	
	auto button2_label = make_shared<GuiLabel>(
		u8_to_unicode( "\xEC\xA1\xB0\xEC\x84\xA0_abcdefghijklmnopqrstuvwxyz" )
	);

	button2_label->dynamic_font_size = true;
	button2_label->style.normal.color_text = glm::vec4{ 0.9f };
	button2_label->style.hover.color_text  = glm::vec4{ 1.0f };
	button2_label->style.normal.padding = label_padding;
	button2_label->style.hover.padding  = label_padding;
	button2->add_child( button2_label );

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



void render_vector_img_item(
	const VectorGraphicsCanvas &canvas,
	const glm::vec4 &canvas_area,
	const ShaderProgram &shader,
	const vector_img::ImgItem *item
)
{
	using namespace vector_img;

	glm::vec2 tmp_a;
	glm::vec2 tmp_b;

	ImgControlPoint const *control_point;
	ImgLine const *line;

	switch( item->type )
	{
		case CONTROL_POINT:
			control_point = static_cast<const ImgControlPoint*>( item );
			tmp_a = {
				canvas_area.x + control_point->x - 2,
				canvas_area.y + control_point->y - 2
			};
			tmp_b = { 5, 5 };
			gl::render_quad_2d( shader, canvas.get_root()->size.to_gl_vec(), tmp_a, tmp_b );
			break;

		case LINE:
			line = static_cast<const ImgLine*>( item );
			tmp_a = glm::vec2{ line->a.x + canvas_area.x, canvas_area.y + line->a.y };
			tmp_b = glm::vec2{ line->b.x + canvas_area.x, canvas_area.y + line->b.y };
			gl::render_line_2d( shader, canvas.get_root()->size.to_gl_vec(), tmp_a, tmp_b );
			break;

		case FILL:
			break;

		default:
			return;
	}
}


glm::vec4 VectorGraphicsCanvas::get_canvas_area() const
{
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

	// Render the image background / area
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

	return glm::vec4 {
		img_area_pos.x,
		img_area_pos.y,
		img_area_size.x,
		img_area_size.y
	};

}


void VectorGraphicsCanvas::render_vector_img() const
{
	auto shader = Globals::shaders.find( "2d" );

	glUseProgram( shader->second.program );

	const auto colorUniform = shader->second.get_uniform( "color" );
	glUniform4f( colorUniform, 1.f, 1.f, 1.f, 0.5f );

	const auto texturedUniform = shader->second.get_uniform( "textured" );
	glUniform1i( texturedUniform, 0 );

	const auto canvas_area   = get_canvas_area();
	const auto img_area_pos  = glm::vec2{ canvas_area.x, canvas_area.y };
	const auto img_area_size = glm::vec2{ canvas_area.z, canvas_area.w };

	gl::render_quad_2d(shader->second, get_root()->size.to_gl_vec(), img_area_pos, img_area_size);

	// Render the image
	for( auto &layer : image.layers )
	{
		if( !layer )
		{
			continue;
		}


		for( auto &item : layer->items )
		{
			if( !item )
			{
				continue;
			}

			render_vector_img_item( *this, canvas_area, shader->second, item.get() );
		}
	}
}

