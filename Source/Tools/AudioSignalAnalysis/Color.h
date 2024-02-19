#pragma once

class Color
{
public:
	Color();
	Color(double r, double g, double b, double a);
	virtual ~Color();

	void Mix(const Color& colorA, const Color& colorB);
	void Invert(const Color& color);

	double r, g, b, a;
};