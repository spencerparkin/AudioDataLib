#include "Math2D.h"
#include <math.h>

//------------------------------ Vector2D ------------------------------

Vector2D::Vector2D()
{
	this->x = 0.0;
	this->y = 0.0;
}

Vector2D::Vector2D(const Vector2D& vector)
{
	this->x = vector.x;
	this->y = vector.y;
}

Vector2D::Vector2D(double x, double y)
{
	this->x = x;
	this->y = y;
}

/*virtual*/ Vector2D::~Vector2D()
{
}

void Vector2D::operator=(const Vector2D& vector)
{
	this->x = vector.x;
	this->y = vector.y;
}

double Vector2D::Length() const
{
	return ::sqrt(this->Dot(*this));
}

Vector2D Vector2D::Normalized() const
{
	return *this / this->Length();
}

double Vector2D::Dot(const Vector2D& vector) const
{
	return this->x * vector.x + this->y * vector.y;
}

double Vector2D::Cross(const Vector2D& vector) const
{
	return this->x * vector.y - this->y * vector.x;
}

Vector2D operator+(const Vector2D& vectorA, const Vector2D& vectorB)
{
	return Vector2D(vectorA.x + vectorB.x, vectorA.y + vectorB.y);
}

Vector2D operator-(const Vector2D& vectorA, const Vector2D& vectorB)
{
	return Vector2D(vectorA.x - vectorB.x, vectorA.y - vectorB.y);
}

Vector2D operator*(const Vector2D& vector, double scalar)
{
	return Vector2D(vector.x * scalar, vector.y * scalar);
}

Vector2D operator*(double scalar, const Vector2D& vector)
{
	return Vector2D(vector.x * scalar, vector.y * scalar);
}

Vector2D operator/(const Vector2D& vector, double scalar)
{
	return Vector2D(vector.x / scalar, vector.y / scalar);
}

//------------------------------ Box2D ------------------------------

Box2D::Box2D()
{
	this->min = Vector2D(0.0, 0.0);
	this->max = Vector2D(0.0, 0.0);
}

Box2D::Box2D(const Box2D& box)
{
	this->min = box.min;
	this->max = box.max;
}

Box2D::Box2D(const Vector2D& min, const Vector2D& max)
{
	this->min = min;
	this->max = max;
}

/*virtual*/ Box2D::~Box2D()
{
}

double Box2D::AspectRatio() const
{
	return this->Width() / this->Height();
}

double Box2D::Area() const
{
	return this->Width() * this->Height();
}
double Box2D::Width() const
{
	return this->max.x - this->min.x;
}

double Box2D::Height() const
{
	return this->max.y - this->min.y;
}

void Box2D::ExpandToMatchAspect(double aspectRatio)
{
	double currentAspectRatio = this->AspectRatio();

	if (currentAspectRatio < aspectRatio)
	{
		double delta = (this->Height() * aspectRatio - this->Width()) / 2.0;
		this->min.x -= delta;
		this->max.x += delta;
	}
	else if (currentAspectRatio > aspectRatio)
	{
		double delta = (this->Width() / aspectRatio - this->Height()) / 2.0;
		this->min.y -= delta;
		this->max.y += delta;
	}
}

void Box2D::ExpandToIncludePoint(const Vector2D& point)
{
	if (this->min.x > point.x)
		this->min.x = point.x;
	if (this->max.x < point.x)
		this->max.x = point.x;
	if (this->min.y > point.y)
		this->min.y = point.y;
	if (this->max.y < point.y)
		this->max.y = point.y;
}

bool Box2D::ContainsPoint(const Vector2D& point) const
{
	return this->min.x <= point.x && point.x <= this->max.x &&
		this->min.y <= point.y && point.y <= this->max.y;
}

bool Box2D::OverlapsWithBox(const Box2D& box) const
{
	return false;
}