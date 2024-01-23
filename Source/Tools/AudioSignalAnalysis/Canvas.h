#pragma once

#include <wx/glcanvas.h>
#include "Math2D.h"

class Canvas : public wxGLCanvas
{
public:
	Canvas(wxWindow* parent);
	virtual ~Canvas();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMouseMotion(wxMouseEvent& event);
	void OnLeftMouseButtonDown(wxMouseEvent& event);
	void OnLeftMouseButtonUp(wxMouseEvent& event);
	void OnMouseWheel(wxMouseEvent& event);
	void OnCaptureLost(wxMouseCaptureLostEvent& event);

	Vector2D MousePosToWorld(const wxPoint& mousePos);
	wxPoint MousePosFromWorld(const Vector2D& worldPoint);

	void FitContent();

private:
	wxGLContext* renderContext;
	static int attributeList[];
	Box2D graphWindow;
	Box2D expandedGraphWindow;
	bool dragging;
	wxPoint lastMousePos;
	double stretchFactor;
};