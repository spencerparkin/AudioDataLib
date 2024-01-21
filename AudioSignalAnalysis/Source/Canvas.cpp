#include "Canvas.h"
#include "App.h"
#include "Audio.h"
#include <gl/GLU.h>

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->renderContext = new wxGLContext(this);

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
	this->Bind(wxEVT_MOUSEWHEEL, &Canvas::OnMouseWheel, this);
	this->Bind(wxEVT_MOTION, &Canvas::OnMouseMotion, this);
	this->Bind(wxEVT_LEFT_DOWN, &Canvas::OnLeftMouseButtonDown, this);
	this->Bind(wxEVT_LEFT_UP, &Canvas::OnLeftMouseButtonUp, this);
	this->Bind(wxEVT_MOUSE_CAPTURE_LOST, &Canvas::OnCaptureLost, this);

	this->graphWindow.min = Vector2D(-10.0, -10.0);
	this->graphWindow.max = Vector2D(10.0, 10.0);

	this->dragging = false;
	this->stretchFactor = 1.0;
}

/*virtual*/ Canvas::~Canvas()
{
	delete this->renderContext;
}

void Canvas::FitContent()
{
	this->graphWindow.min = Vector2D(0.0, 0.0);
	this->graphWindow.max = Vector2D(0.0, 0.0);

	for (const Audio* audio : wxGetApp().audioArray)
	{
		Box2D boundingBox = audio->CalcBoundingBox();
		this->graphWindow.ExpandToIncludeBox(boundingBox);
	}
}

void Canvas::OnPaint(wxPaintEvent& event)
{
	this->SetCurrent(*this->renderContext);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	double aspectRatio = float(viewport[2]) / float(viewport[3]);

	this->expandedGraphWindow = this->graphWindow;
	this->expandedGraphWindow.ExpandToMatchAspect(aspectRatio);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(
		expandedGraphWindow.min.x * this->stretchFactor,
		expandedGraphWindow.max.x * this->stretchFactor,
		expandedGraphWindow.min.y,
		expandedGraphWindow.max.y);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_LINES);

	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex2f(-20.0, 0.0f);
	glVertex2f(20.0, 0.0f);

	glVertex2f(0.0f, -20.0);
	glVertex2f(0.0f, 20.0);

	for (uint32_t i = 1; i < 20; i++)
	{
		GLfloat tickSize = ((i % 5) == 0) ? 0.2f : 0.1f;

		if (i % 5 == 0)
			glColor3f(0.0f, 0.0f, 1.0f);
		else
			glColor3f(1.0f, 1.0f, 1.0f);

		glVertex2f(GLfloat(i), -tickSize);
		glVertex2f(GLfloat(i), tickSize);

		glVertex2f(-GLfloat(i), -tickSize);
		glVertex2f(-GLfloat(i), tickSize);

		glVertex2f(-tickSize, GLfloat(i));
		glVertex2f(tickSize, GLfloat(i));

		glVertex2f(-tickSize, -GLfloat(i));
		glVertex2f(tickSize, -GLfloat(i));
	}

	glEnd();

	for (const Audio* audio : wxGetApp().audioArray)
		audio->Render();

	glFlush();

	this->SwapBuffers();
}

void Canvas::OnSize(wxSizeEvent& event)
{
	this->SetCurrent(*this->renderContext);

	wxSize size = event.GetSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());

	this->Refresh();
}

void Canvas::OnMouseMotion(wxMouseEvent& event)
{
	if (this->dragging)
	{
		wxPoint currentMousePos = event.GetPosition();
		wxPoint mouseDelta = currentMousePos - this->lastMousePos;
		this->lastMousePos = currentMousePos;
		Vector2D dragDelta(-mouseDelta.x, mouseDelta.y);
		double mouseSensativity = 0.001 * this->graphWindow.Width();
		dragDelta *= mouseSensativity;
		this->graphWindow.min += dragDelta;
		this->graphWindow.max += dragDelta;
		this->Refresh();
	}
}

void Canvas::OnLeftMouseButtonDown(wxMouseEvent& event)
{
	this->dragging = true;
	this->CaptureMouse();
	this->lastMousePos = event.GetPosition();
}

void Canvas::OnLeftMouseButtonUp(wxMouseEvent& event)
{
	if (this->dragging)
	{
		this->dragging = false;
		this->ReleaseMouse();
	}
}

void Canvas::OnMouseWheel(wxMouseEvent& event)
{
	double wheelChange = double(event.GetWheelRotation() / 120);
	wxPoint currentMousePos = event.GetPosition();
	constexpr double wheelSensativity = 0.05;
	double zoomFactor = wheelChange * wheelSensativity;
	if (event.ShiftDown())
		this->stretchFactor *= 1.0 + wheelChange / 10.0;
	else
	{
		Vector2D mousePoint = this->MousePosToWorld(event.GetPosition());
		this->graphWindow.Zoom(mousePoint, zoomFactor);
	}
	this->Refresh();
}

void Canvas::OnCaptureLost(wxMouseCaptureLostEvent& event)
{
	this->dragging = false;
}

Vector2D Canvas::MousePosToWorld(const wxPoint& mousePos)
{
	wxSize clientSize = this->GetClientSize();
	Box2D clientBox(Vector2D(0.0, 0.0), Vector2D(clientSize.x, clientSize.y));
	Vector2D mousePoint(mousePos.x, clientSize.y - mousePos.y);
	Vector2D worldPoint = this->expandedGraphWindow.FromUVs(clientBox.ToUVs(mousePoint));
	return worldPoint;
}

wxPoint Canvas::MousePosFromWorld(const Vector2D& worldPoint)
{
	wxSize clientSize = this->GetClientSize();
	Box2D clientBox(Vector2D(0.0, 0.0), Vector2D(clientSize.x, clientSize.y));
	Vector2D mousePoint = clientBox.FromUVs(this->expandedGraphWindow.ToUVs(worldPoint));
	wxPoint mousePos(int(mousePoint.x), clientSize.y - int(mousePoint.y));
	return mousePos;
}