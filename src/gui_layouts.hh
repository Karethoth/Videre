#pragma once
#include "gui.hh"
#include <vector>
#include <functional>

namespace gui
{
struct GridLayout;
struct SplitLayout;

using GuiElementPtrPair = std::pair<GuiElementPtr, GuiElementPtr>;

enum SplitAxis {
	UNDEFINED_AXIS,
	HORIZONTAL,
	VERTICAL,
	HORIZONTAL_AND_VERTICAL
};


struct GridLayout : GuiElement
{
	const size_t rows;
	const size_t columns;

	std::vector<GuiPixelsOrPercentage> col_widths;
	std::vector<GuiPixelsOrPercentage> row_heights;

	GridLayout( size_t _rows = 1, size_t _columns = 1 );
	virtual ~GridLayout();

	virtual void handle_event( const GuiEvent &e ) override;
	virtual void render() const override;
	virtual GuiVec2 get_minimum_size() const override;

  protected:
	int auto_width;
	int auto_height;
	int used_width;
	int used_height;

	void update_dimensions();
	void fit_children();
};



struct SplitLayout : GuiElement
{
	static const int minimum_window_size = 20;
	static const int border_touch_width = 10;
	static const int border_drag_width = 5;


	// Function that creates the children upon a split
	std::function<GuiElementPtrPair()> create_children =
		[] { return GuiElementPtrPair (
				std::make_shared<SplitLayout>(),
				std::make_shared<SplitLayout>() ); };

	SplitAxis split_axis = UNDEFINED_AXIS;
	SplitAxis allowed_axes = HORIZONTAL_AND_VERTICAL;
	bool is_layout_splitted = false;
	bool is_splitting_allowed = true;

	struct
	{
		SplitAxis axis    = UNDEFINED_AXIS;
		int offset        = 0;
		float ratio       = 0.5f;
		bool is_visible   = false;
		bool is_hilighted = false;
		bool is_dragged   = false;
		bool is_locked    = false;
	} split_bar;

	virtual ~SplitLayout();

	virtual void handle_event( const GuiEvent &e ) override;
	virtual void render() const override;

	virtual GuiVec2 get_minimum_size() const override;
	virtual void split_at( SplitAxis axis, int offset );

  protected:
	void split_layout();
	void fit_split_bar();
	void fit_children();

	virtual void init_child( GuiElement *child ) override;

	// Adding children from outside doesn't make sense
	using GuiElement::add_child;
};

} // namespace gui

