#pragma once
#include "gui.hh"
#include <vector>

namespace gui
{

struct GridLayout : GuiElement
{
	const int rows;
	const int columns;

	std::vector<GuiPixelsOrPercentage> row_sizes;
	std::vector<GuiPixelsOrPercentage> col_sizes;

	GridLayout( int _rows = 1, int _columns = 1 );

	virtual void handle_event( const GuiEvent &e ) override;
};


enum SplitAxis { UNDEFINED_AXIS, HORIZONTAL, VERTICAL };

struct SplitLayout : GuiElement
{
	static const int minimum_window_size = 20;
	static const int border_touch_width = 10;
	static const int border_drag_width = 5;

	SplitAxis split_axis = UNDEFINED_AXIS;
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

	virtual void handle_event( const GuiEvent &e ) override;
	virtual void render() const override;

	virtual GuiVec2 get_minimum_size() const override;


  protected:
	void split_layout();
	void fit_split_bar();
	void fit_children();

	virtual void init_child( GuiElement *child ) override;
};

} // namespace gui

