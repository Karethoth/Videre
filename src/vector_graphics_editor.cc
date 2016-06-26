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
: toolbar_element( nullptr ),
  canvas_element( nullptr ),
  properties_element( nullptr )
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
: image( vector_img::VectorImg{} )
{
	// Set up the image
	image.img_w = 320;
	image.img_h = 240;
	auto new_layer = make_unique<vector_img::ImgLayer>();
	image.layers.emplace_back( move( new_layer ) );

	camera_offset = { 0, 0 };
	scale = 1.f;
}



void VectorGraphicsCanvas::handle_event( const gui::GuiEvent &e )
{
	// TODO: Decide if context menu should or should not close
	//       when mouse is used in an another element
	const auto close_when_moused_elsewhere = false;

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
			auto window = dynamic_cast<Window*>( get_root() );
			if( window )
			{
				window->clear_popups();
			}
		}
	}
	else if( e.type == MOUSE_BUTTON )
	{
		const auto is_in_area = in_area( e.mouse_button.pos );
		
		if( (close_when_moused_elsewhere || is_in_area) &&
		    (e.mouse_button.button != 3 || e.mouse_button.state != RELEASED) )
		{
			auto window = dynamic_cast<Window*>( get_root() );
			if( window )
			{
				window->clear_popups();
			}
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

	window->clear_popups();

	auto popup_menu = make_shared<PopupElement>();
	popup_menu->style.normal.color_bg = { 0.8, 0.8, 0.8, 0.6 };
	popup_menu->style.hover.color_bg = { 0.8, 0.8, 0.8, 0.6 };
	popup_menu->target_pos = tgt_pos;

	auto menu = make_shared<Menu>();

	// Create a test button
	const auto label_padding = glm::vec4( 6.f, 6.f, 6.f, 6.f );

	auto button1 = make_shared<GuiContextButton<PopupElementContext>>();
	button1->context.popup = popup_menu.get();
	button1->style.normal.color_bg = { 0.0, 0.0, 0.0, 0.8 };
	button1->style.hover.color_bg  = { 0.0, 0.0, 0.0, 1.0 };
	button1->on_click = [&image=image, this]( GuiElement *element, const GuiEvent &e )
	{
		auto window = dynamic_cast<Window*>( element->get_root() );
		auto button = dynamic_cast<GuiContextButton<PopupElementContext>*>( element );
		if( e.mouse_button.button != 1 ||
			e.mouse_button.state != RELEASED ||
			!element->in_area( e.mouse_button.pos ) )
		{
			return;
		}

		const auto canvas_area = get_canvas_area();

		auto item = make_unique<vector_img::ImgControlPoint>();
		item->x = button->context.popup->target_pos.x - canvas_area.x;
		item->y = button->context.popup->target_pos.y - canvas_area.y;
		image.layers[0]->items.push_back( move( item ) );
		item.reset();

		window->remove_popup( button->context.popup );
	};
	
	auto button1_label = make_shared<GuiLabel>(
		u8_to_unicode( "Just testing" )
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
	auto button2 = make_shared<GuiContextButton<PopupElementContext>>();
	button2->context.popup = popup_menu.get();
	button2->style.normal.color_bg = { 0.0, 0.0, 0.0, 0.8 };
	button2->style.hover.color_bg  = { 0.0, 0.0, 0.0, 1.0 };
	button2->on_click = [&image=image, this]( GuiElement *element, const GuiEvent &e )
	{
		auto window = dynamic_cast<Window*>( element->get_root() );
		auto button = dynamic_cast<GuiContextButton<PopupElementContext>*>( element );
		if( e.mouse_button.button != 1 ||
			e.mouse_button.state != RELEASED ||
			!element->in_area( e.mouse_button.pos ) )
		{
			return;
		}

		const auto canvas_area = get_canvas_area();

		auto item = make_unique<vector_img::ImgControlPoint>();
		item->x = button->context.popup->target_pos.x - canvas_area.x;
		item->y = button->context.popup->target_pos.y - canvas_area.y;
		image.layers[0]->items.push_back( move( item ) );
		item.reset();

		window->remove_popup( button->context.popup );
	};
	
	auto button2_label = make_shared<GuiLabel>(
		u8_to_unicode( "\xEC\xA1\xB0\xEC\x84\xA0O123" )
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

	window->add_popup( popup_menu );
	
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

	const ImgControlPoint *control_point = nullptr;
	const ImgLine *line = nullptr;

	switch( item->type )
	{
		case CONTROL_POINT:
			control_point = dynamic_cast<const ImgControlPoint*>( item );
			tmp_a = {
				canvas_area.x + control_point->x - 2,
				canvas_area.y + control_point->y - 2
			};
			tmp_b = { 5, 5 };
			gl::render_quad_2d( shader, canvas.get_root()->size.to_gl_vec(), tmp_a, tmp_b );
			break;

		case LINE:
			line = dynamic_cast<const ImgLine*>( item );
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
		img_area_pos.x = tools::int_to_float( pos.x );
	}

	if( img_pos.y < pos.y  )
	{
		img_area_pos.y = tools::int_to_float( pos.y );
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


	auto area = glm::vec4();
	area.x = img_area_pos.x;
	area.y = img_area_pos.y;
	area.z = img_area_size.x;
	area.w = img_area_size.y;

	return area;
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



VectorGraphicsToolbar::VectorGraphicsToolbar()
{
	// Create button
	auto button = make_shared<GuiButton>();
	button->style.normal.color_bg = { 0.0f, 0.0, 0.0, 0.0 };
	button->style.hover.color_bg  = { 1.0f, 1.0, 1.0, 0.05 };

	auto button_label = make_shared<GuiLabel>(
		u8_to_unicode( "子猫は月光です-1234-１２３４"),
		16
	);
	auto label_padding = glm::vec4( 4, 4, 4, 0 );
	button_label->style.normal.color_text = glm::vec4{ 0.8f };
	button_label->style.hover.color_text  = glm::vec4{ 1.f };
	button_label->style.normal.padding = label_padding;
	button_label->style.hover.padding  = label_padding;
	button_label->dynamic_font_size    = false;

	button->on_click = []( GuiElement *tgt, const GuiEvent &e )
	{
		if( e.type != MOUSE_BUTTON ||
			e.mouse_button.state != RELEASED ||
		    !tgt->in_area( e.mouse_button.pos ) )
		{
			return;
		}

		auto window = dynamic_cast<Window*>(tgt->get_root());
		if( !window )
		{
			return;
		}

		window->clear_popups();

		auto popup_menu = make_shared<PopupElement>();
		popup_menu->style.normal.color_bg = { 0.15, 0.15, 0.15, 1.0 };
		popup_menu->style.hover.color_bg  = { 0.15, 0.15, 0.15, 1.0 };
		popup_menu->target_pos    = tgt->pos;
		popup_menu->target_pos.y += tgt->size.h;

		auto label = make_shared<GuiLabel>(
			u8_to_unicode( "Nothing to see here" )
		);
		label->style.normal.color_text = { 1.0, 1.0, 1.0, 0.4 };
		label->style.hover.color_text  = { 1.0, 1.0, 1.0, 0.5 };
		label->style.normal.padding    = { 6, 8, 8, 8 };
		label->style.hover.padding     = { 6, 8, 8, 8 };
		label->dynamic_font_size       = true;
		popup_menu->add_child( label );
		popup_menu->parent = window;
		window->add_popup( popup_menu );

		// Resize the context menu
		GuiEvent event;
		event.type = MOVE;
		event.move.pos = tgt->pos;
		event.move.pos.y += tgt->size.h;
		popup_menu->handle_event( event );

		event.type = RESIZE;
		event.resize.size = { 100, 0 };
		popup_menu->handle_event( event );
	};

	button->add_child( button_label );
	add_child( button );
}



void VectorGraphicsToolbar::handle_event( const GuiEvent &e )
{
	if( e.type == RESIZE )
	{
		size = e.resize.size;
		fit_children();
	}
	else
	{
		GuiElement::handle_event( e );
	}
}



void VectorGraphicsToolbar::fit_children()
{
	GuiEvent event;

	auto element_position_x = pos.x;
	for( auto &child : children )
	{
		const auto child_min_size = child->get_minimum_size();

		event.type = MOVE;
		event.move.pos = { element_position_x, pos.y };
		child->handle_event( event );

		event.type = RESIZE;
		event.resize.size = { child_min_size.w, size.h };
		child->handle_event( event );

		element_position_x += child_min_size.w;
	}
}

