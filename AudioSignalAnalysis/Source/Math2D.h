#pragma once

class Vector2D
{
public:
	Vector2D();
	Vector2D(const Vector2D& vector);
	Vector2D(double x, double y);
	virtual ~Vector2D();

	void operator=(const Vector2D& vector);
	void operator+=(const Vector2D& vector);
	void operator-=(const Vector2D& vector);
	void operator*=(double scalar);
	void operator/=(double scalar);

	double Length() const;
	Vector2D Normalized() const;
	double Dot(const Vector2D& vector) const;
	double Cross(const Vector2D& vector) const;

	double x, y;
};

Vector2D operator+(const Vector2D& vectorA, const Vector2D& vectorB);
Vector2D operator-(const Vector2D& vectorA, const Vector2D& vectorB);
Vector2D operator*(const Vector2D& vector, double scalar);
Vector2D operator*(double scalar, const Vector2D& vector);
Vector2D operator/(const Vector2D& vector, double scalar);

class Box2D
{
public:
	Box2D();
	Box2D(const Box2D& box);
	Box2D(const Vector2D& min, const Vector2D& max);
	virtual ~Box2D();

	double AspectRatio() const;
	double Area() const;
	double Width() const;
	double Height() const;
	Vector2D Center() const;
	void ExpandToMatchAspect(double aspectRatio);
	void ExpandToIncludePoint(const Vector2D& point);
	void ExpandToIncludeBox(const Box2D& box);
	bool ContainsPoint(const Vector2D& point) const;
	bool OverlapsWithBox(const Box2D& box) const;
	Vector2D FromUVs(const Vector2D& uvs) const;
	Vector2D ToUVs(const Vector2D& point) const;
	void Zoom(const Vector2D& focalPoint, double zoomFactor);

	Vector2D min, max;
};