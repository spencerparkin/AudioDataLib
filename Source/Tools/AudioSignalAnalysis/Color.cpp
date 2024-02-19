#include "Color.h"

Color::Color()
{
	this->r = 1.0;
	this->g = 1.0;
	this->b = 1.0;
	this->a = 0.0;
}

Color::Color(double r, double g, double b, double a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

/*virtual*/ Color::~Color()
{
}

void Color::Mix(const Color& colorA, const Color& colorB)
{
	this->r = colorA.r + colorB.r;
	this->g = colorA.g + colorB.g;
	this->b = colorA.b + colorB.b;
	this->a = colorA.a + colorB.a;

	if (this->r > 1.0)
		this->r = 1.0;

	if (this->g > 1.0)
		this->g = 1.0;

	if (this->b > 1.0)
		this->b = 1.0;

	if (this->a > 1.0)
		this->a = 1.0;
}