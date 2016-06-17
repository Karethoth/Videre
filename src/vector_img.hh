#pragma once
#include <vector>
#include <memory>

namespace vector_img
{


enum ImgItemType : uint8_t
{
	NO_TYPE,
	CONTROL_POINT,
	LINE,
	FILL
};


struct ImgItem;
struct ImgLayer;

using ImgItemPtr  = std::unique_ptr<ImgItem>;
using ImgLayerPtr = std::unique_ptr<ImgLayer>;


struct Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};



struct VectorImg
{
	size_t img_w;
	size_t img_h;

	std::vector<ImgLayerPtr> layers;
};



struct ImgLayer
{
	std::vector<ImgItemPtr> items;
};



struct ImgItem
{
	ImgItemType type;

	float x;
	float y;

	Color color;

	ImgItem();
};



struct ImgControlPoint : ImgItem
{
	ImgControlPoint();
};



struct ImgLine : ImgItem
{
	ImgControlPoint a;
	ImgControlPoint b;
	float width;

	ImgLine();
};


struct ImgFill : ImgItem
{
	ImgFill();
};


};
