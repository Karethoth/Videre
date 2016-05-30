#pragma once

#include "vector_img.hh"

using namespace vector_img;


ImgItem::ImgItem()
{
	type = ImgItemType::NO_TYPE;
}



ImgControlPoint::ImgControlPoint()
{
	type = ImgItemType::CONTROL_POINT;
}



ImgLine::ImgLine()
{
	type = ImgItemType::LINE;
}



ImgFill::ImgFill()
{
	type = ImgItemType::FILL;
}

