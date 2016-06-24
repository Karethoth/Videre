#pragma once

#include "vector_img.hh"

using namespace vector_img;


VectorImg::VectorImg()
: img_w(0),
  img_h(0)
{
}



ImgItem::ImgItem()
: type( ImgItemType::NO_TYPE ),
  x( 0.0 ),
  y( 0.0 ),
  color( Color{ 255, 255, 255, 255 } )
{
}



ImgControlPoint::ImgControlPoint()
{
	type = ImgItemType::CONTROL_POINT;
}



ImgLine::ImgLine()
: width( 0 )
{
	type = ImgItemType::LINE;
}



ImgFill::ImgFill()
{
	type = ImgItemType::FILL;
}

