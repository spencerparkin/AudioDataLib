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

	this->graphWindow.min = Vector2D(-10.0, -10.0);
	this->graphWindow.max = Vector2D(10.0, 10.0);
}

/*virtual*/ Canvas::~Canvas()
{
	delete this->renderContext;
}

void Canvas::OnPaint(wxPaintEvent& event)
{
	this->SetCurrent(*this->renderContext);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	double aspectRatio = float(viewport[2]) / float(viewport[3]);

	Box2D expandedGraphWindow(this->graphWindow);
	expandedGraphWindow.ExpandToMatchAspect(aspectRatio);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(expandedGraphWindow.min.x, expandedGraphWindow.max.x, expandedGraphWindow.min.y, expandedGraphWindow.max.y);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_LINES);

	glColor3f(0.5f, 0.5f, 0.5f);

	glVertex2f(this->graphWindow.min.x, 0.0f);
	glVertex2f(this->graphWindow.max.x, 0.0f);

	glVertex2f(0.0f, this->graphWindow.min.y);
	glVertex2f(0.0f, this->graphWindow.max.y);

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