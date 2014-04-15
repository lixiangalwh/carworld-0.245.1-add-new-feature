
#include "H_Standard.h"

#include <math.h>
#include <map>

#include <string>
#include "H_Graphics.h"
#include "H_Texture.h"
#include <SDL.h>

Hgl *Hgl::curr = NULL;

const Color Black(0,0,0);
const Color White(255,255,255);
const Color Grey(126,126,126);


static GLubyte rasters[][13] = {
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36},
{0x00, 0x00, 0x00, 0x66, 0x66, 0xff, 0x66, 0x66, 0xff, 0x66, 0x66, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x7e, 0xff, 0x1b, 0x1f, 0x7e, 0xf8, 0xd8, 0xff, 0x7e, 0x18},
{0x00, 0x00, 0x0e, 0x1b, 0xdb, 0x6e, 0x30, 0x18, 0x0c, 0x76, 0xdb, 0xd8, 0x70},
{0x00, 0x00, 0x7f, 0xc6, 0xcf, 0xd8, 0x70, 0x70, 0xd8, 0xcc, 0xcc, 0x6c, 0x38},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x0c, 0x0e},
{0x00, 0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c},
{0x00, 0x00, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x30},
{0x00, 0x00, 0x00, 0x00, 0x99, 0x5a, 0x3c, 0xff, 0x3c, 0x5a, 0x99, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03},
{0x00, 0x00, 0x3c, 0x66, 0xc3, 0xe3, 0xf3, 0xdb, 0xcf, 0xc7, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x38, 0x18},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0x07, 0x03, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xff, 0xcc, 0x6c, 0x3c, 0x1c, 0x0c},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x03, 0x7f, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x1c, 0x1c, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06},
{0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60},
{0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x0c, 0x06, 0x03, 0xc3, 0xc3, 0x7e},
{0x00, 0x00, 0x3f, 0x60, 0xcf, 0xdb, 0xd3, 0xdd, 0xc3, 0x7e, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18},
{0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e},
{0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3},
{0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c},
{0x00, 0x03, 0x03, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60},
{0x00, 0x00, 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18},
{0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x38, 0x30, 0x70},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0x7f, 0x03, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0xc3, 0xc0, 0xc0, 0xc0, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x03, 0x03, 0x03, 0x03, 0x03},
{0x00, 0x00, 0x7f, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x33, 0x1e},
{0x7e, 0xc3, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
{0x38, 0x6c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x00},
{0x00, 0x00, 0xc6, 0xcc, 0xf8, 0xf0, 0xd8, 0xcc, 0xc6, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78},
{0x00, 0x00, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfc, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x03, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0x03, 0x03, 0x7e, 0xc0, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x1c, 0x36, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x00},
{0x00, 0x00, 0x7e, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xdb, 0xc3, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0x60, 0x60, 0x30, 0x18, 0x3c, 0x66, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xff, 0x60, 0x30, 0x18, 0x0c, 0x06, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x0f, 0x18, 0x18, 0x18, 0x38, 0xf0, 0x38, 0x18, 0x18, 0x18, 0x0f},
{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0xf0, 0x18, 0x18, 0x18, 0x1c, 0x0f, 0x1c, 0x18, 0x18, 0x18, 0xf0},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x8f, 0xf1, 0x60, 0x00, 0x00, 0x00}
};



//CLASS Color
void level(Color &A)
{
	bool OverR=false, OverG=false;
	if (A.x()>1.)
	{
		A.y() += (A.x()-1.f)*0.5f;
		A.z() += (A.x()-1.f)*0.5f;
		OverR=true;
		A.x()=1.f;
	}
	if (A.y()>254)
	{
		if (!OverR) A.x() += (A.y()-1.f)*0.5f;
		A.z() += (A.y()-1.f)*0.5f;
		OverG=true;
		A.y()=1.f;
	}
	if (A.z()>1.)
	{
		if (!OverG) A.y() += (A.z()-1.f)*0.5f;
		if (!OverR) A.x() += (A.z()-1.f)*0.5f;
		A.z()=1.;
	}
}

//CLASS Hgl
Hgl::Hgl(HWindow *w) :
	m_window(w),
	glLockArrays(NULL),
	glUnlockArrays(NULL),
	fontOffset(0)
{
	MakeCurrent();
//initialize the function pointers to the glExtentions:

	glLockArrays = (PFNGLLOCKARRAYSEXTPROC) SDL_GL_GetProcAddress("glLockArraysEXT");
	if (glLockArrays != NULL)
		cout << "glLockArrays initiated...\n";

	glUnlockArrays = (PFNGLUNLOCKARRAYSEXTPROC) SDL_GL_GetProcAddress("glUnlockArraysEXT");
	if (glUnlockArrays != NULL)
		cout << "glUnlockArrays initiated...\n";

	//get gl extentions string
	curr->GLExtentions = Command((char*)glGetString(GL_EXTENSIONS));

	PrintVersion(cout);

	cout << endl;

	//set hgl defaults
 	Hgl::Disable(Hgl::SHADOW);
	Hgl::SetShadows(HGL_SHADOWS_DEFAULT);
 	Hgl::SetTextureMode(HGL_TEXTURE_MODE_DEFAULT);
 	Hgl::SetFinish(HGL_FINISH_DEFAULT);
 	Hgl::SetVertexArrays(HGL_VERTEX_ARRAYS_DEFAULT);
 	Hgl::SetExtCompiledVertexArrays((glLockArrays!=NULL) && HGL_EXT_COMPILED_VERTEX_ARRAYS_DEFAULT);
 
	//set gl defaults
	glLoadIdentity();
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
	glEnable(GL_CULL_FACE);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glFrontFace(GL_CW);

	ThrowError();
}

void Hgl::MakeCurrent()
{
	m_window->MakeCurrent();
	curr = this;
}
void Hgl::SwapBuffers()
{
	curr->m_window->SwapBuffers();
}
Hgl &Hgl::GetCurrent()
{
	return *curr;
}

map<string,Texture::Reference> &Hgl::GetLoadedTextures()
{
	return curr->LoadedTextures;
}

void Hgl::ClearColor(const Color &AColor)
{
    glClearColor(AColor.x(), AColor.y(), AColor.z(), 0.0f);
}

ostream &Hgl::PrintVersion(ostream &out)
{
	out << "OpenGL renderer:  " << (const char*)glGetString(GL_RENDERER) << endl;
	out << "OpenGL renderer vendor:  " << (const char*)glGetString(GL_VENDOR) << endl;
	out << "OpenGL renderer version: " << (const char*)glGetString(GL_VERSION) << endl;
	return out;
}

ostream &Hgl::PrintExtentions(ostream &out)
{
	out << "supported openGL extentions:\n";
	for (unsigned int i=0 ; i<curr->GLExtentions.size() ; i++)
		out << "    " << curr->GLExtentions[i].c_str() << "\n";
	return out;
}

void Hgl::ResizeWindow(HRect &ClientRect)
{
	curr->MyViewPort = ClientRect;
	if (ClientRect.GetHeight()>0)
	{
		cout << "resize: " << (int)ClientRect.GetWidth() << "x" << (int)ClientRect.GetHeight() << "\n";
		Hgl::Viewport(ClientRect);
		REAL Aspect = ((REAL)ClientRect.GetWidth())/ClientRect.GetHeight();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(FIELD_OF_VIEW, Aspect, NEAR_PLANE, FAR_PLANE);
		glMatrixMode(GL_MODELVIEW);
	}
}

HRect Hgl::GetViewPort()
{
	return curr->MyViewPort;
}

static GLuint makeRasterFont()
{
	GLuint font;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    font = glGenLists(128);
    for (GLuint i = 32; i < 127; i++) {
        glNewList(i+font, GL_COMPILE);
        glBitmap(FONT_WIDTH, FONT_HEIGHT, 0.0, 0.0, FONT_WIDTH+2, 0.0, rasters[i-32]);
        glEndList();
    }
	//restor unpack alignment back to 4 (default value)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return font;
}

static void t_chaine(GLuint& font, const char* s)
{
	if (font==0)
		font = makeRasterFont();

	//glPushAttrib (GL_LIST_BIT);
	glListBase(font);
	glCallLists(strlen(s), GL_UNSIGNED_BYTE, (const GLvoid *)s);
	//glPopAttrib ();
	glListBase(0);
}

void Hgl::WriteText(const char* AText, const Point<3,GLfloat> &APosition)
{
	//glRasterPos3fv(APosition.p);
	glRasterPos3f(APosition.x(),APosition.y(),APosition.z());
	t_chaine(curr->fontOffset,AText);
}
void Hgl::WriteText(const char* AText, const Point<2,GLfloat> &APosition)
{
	glRasterPos2f(APosition.x(),APosition.y());
	t_chaine(curr->fontOffset,AText);
}
void Hgl::Translate(const Point<3,GLfloat> &Location)
{
	glTranslatef(Location.x(), Location.y(), Location.z());
}
void Hgl::Rotate(REAL teta, const Point<3,GLfloat> &Axe)
{
	glRotatef(teta*(180/PI), Axe.x(), Axe.y(), Axe.z());
}
void Hgl::LookFrom(const Ref & ARef)
{
	//the camera looks from the origine of ARef
	//down the Y axis
	//useing Z as the up vector
	gluLookAt(ARef.Position.x(),
		ARef.Position.y(),
		ARef.Position.z(),
		ARef.Position.x() + ARef.Y.x(),
		ARef.Position.y() + ARef.Y.y(),
		ARef.Position.z() + ARef.Y.z(),
		ARef.Z.x(),
		ARef.Z.y(),
		ARef.Z.z()
	);
}
void Hgl::Relocate(const Ref &Position)
{
	//Hgl::Relocate(Position.Position, Position.Y, Position.Z);
	static GLfloat m[16];
	//Point<3,GLfloat> W = Direction^Up;
	//colum 1:
	m[0] = Position.X.x(); m[1] = Position.X.y(); m[2] = Position.X.z(); m[3] = 0;
	//colum 2:
	m[4] = Position.Y.x(); m[5] = Position.Y.y(); m[6] = Position.Y.z(); m[7] = 0;
	//colum 3:
	m[8] = Position.Z.x(); m[9] = Position.Z.y(); m[10] = Position.Z.z(); m[11] = 0;
	//colum 4:
	m[12] = Position.Position.x(); m[13] = Position.Position.y(); m[14] = Position.Position.z(); m[15] = 1;

	glMultMatrixf(m);
}
void Hgl::Finish()
{
	if (curr->gl_finish)
		glFinish();
}
void Hgl::SetLightSource(Point<3,GLfloat> LightDirection)
{
	float LightParams[4];
	LightDirection.normalize();
	LightParams[0] = (float)LightDirection.x();
	LightParams[1] = (float)LightDirection.y();
	LightParams[2] = (float)LightDirection.z();
	LightParams[3] = 0;
	glLightfv(GL_LIGHT0,
			GL_POSITION,
			LightParams
			);
}
void Hgl::Viewport(const HRect &ClientRect)
{
	glViewport((int)ClientRect.left,(int)ClientRect.top,(int)ClientRect.right,(int)ClientRect.bottom);
}

#define X 0
#define Y 1
#define Z 2
#define W 3

/* print a matrix */
ostream & operator << (ostream & outfile, float Matrix[4][4])
{
	for (int i=0 ; i<4 ; i++)
	{
		for (int j=0 ; j<4 ; j++)
			outfile << Matrix[i][j] << " ";
		outfile << "\n";
	}
	return outfile;
}

/* Create a matrix that will project the desired shadow. */
static void shadowMatrix(REAL shadowMat[4][4],
  REAL groundplane[4],
  REAL lightpos[4])
{
  /* Find dot product between light position vector and ground plane normal. */
	REAL dot = groundplane[X] * lightpos[X] +
		groundplane[Y] * lightpos[Y] +
		groundplane[Z] * lightpos[Z] +
		groundplane[W] * lightpos[W];

	shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
	shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
	shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
	shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];

	shadowMat[0][1] = 0.f - lightpos[Y] * groundplane[X];
	shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
	shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
	shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];

	shadowMat[0][2] = 0.f - lightpos[Z] * groundplane[X];
	shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
	shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
	shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];

	shadowMat[0][3] = 0.f - lightpos[W] * groundplane[X];
	shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
	shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
	shadowMat[3][3] = dot - lightpos[W] * groundplane[W];
	
	//CERR << shadowMat;
}

/* Find the plane equation given 3 points. */
static void findPlane(REAL plane[4], Point<3,GLfloat> &v0, Point<3,GLfloat> &v1, Point<3,GLfloat> &v2)
{
  /* Need 2 vectors to find cross product. */
  Point<3,GLfloat> vec0 = v1 - v0;
  Point<3,GLfloat> vec1 = v2 - v0;
  Point<3,GLfloat> VecProd = vec0^vec1;

  /* find cross product to get A, B, and C of plane equation */
  plane[0] = VecProd.x();
  plane[1] = VecProd.y();
  plane[2] = VecProd.z();

  plane[3] = -(plane[0] * v0.x() + plane[1] * v0.y() + plane[2] * v0.z());
}

void Hgl::ShadowTransform(const Point<3,GLfloat> &LightPos, const Contact &ShadowContact)
{
	REAL ShadowMat[4][4];
	
	REAL lightpos[4];
	lightpos[0] = LightPos.x();
	lightpos[1] = LightPos.y();
	lightpos[2] = LightPos.z();
	lightpos[3] = 0;

	REAL plane[4];
	plane[0] = ShadowContact.Normal.x();
	plane[1] = ShadowContact.Normal.y();
	plane[2] = ShadowContact.Normal.z();
	plane[3] = -(plane[0] * ShadowContact.Position.x() + plane[1]
		*ShadowContact.Position.y() + plane[2] * ShadowContact.Position.z());

	shadowMatrix(ShadowMat, plane, lightpos);
	glMultMatrixf((GLfloat*)ShadowMat);
}
void Hgl::ShadowTransform(const Point<3,GLfloat> &LightPos, Point<3,GLfloat> APlane[3])
{
	REAL ShadowMat[4][4];
	REAL plane[4];
	
	REAL lightpos[4];
	lightpos[0] = LightPos.x();
	lightpos[1] = LightPos.y();
	lightpos[2] = LightPos.z();
	lightpos[3] = 0;

	findPlane(plane, APlane[0], APlane[1], APlane[2]);
	shadowMatrix(ShadowMat, plane, lightpos);
	glMultMatrixf((GLfloat *) ShadowMat);
}

void Hgl::ThrowError()
{
//BUG glGetError() is only defined from OpenGL 1.2 on so we need to test ...
	//GLenum ErrorType = glGetError();
	//if (ErrorType!=GL_NO_ERROR)
	//	throw HException(string("opengl error: ")+(const char *)gluErrorString(ErrorType));
}

void Hgl::PurgeError()
{
//BUG glGetError() is only defined from OpenGL 1.2 on so we need to test ...
//#ifdef
//	while (glGetError() != GL_NO_ERROR);
//#endif 
}

ostream &Hgl::PrintDebug(ostream &out)
{
	GLfloat m[16];
	int i,j;
	glGetFloatv(GL_MODELVIEW_MATRIX,m);
	out << "modelview matrix:\n";
	for (j=0 ; j<4 ; j++)
	{
		for (i=0 ; i<4 ; i++)
			out << m[4*i+j] << " ";
		out << endl;
	}
	glGetFloatv(GL_PROJECTION_MATRIX,m);
	out << "projection matrix:\n";
	for (j=0 ; j<4 ; j++)
	{
		for (i=0 ; i<4 ; i++)
			out << m[4*i+j] << " ";
		out << endl;
	}
	return out;
}

void Hgl::LockArrays(int first, int count)
{
	if (curr->gl_ext_compiled_vertex_array)
		(*curr->glLockArrays)(first, count);
}

void Hgl::UnlockArrays()
{
	if (curr->gl_ext_compiled_vertex_array)
		(*curr->glUnlockArrays)();
}

void Hgl::Enable(Enum variable)
{
	GetCapEnable(variable) = true;
	if (variable == Hgl::SHADOW)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		if (Hgl::GetShadows() == 2)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_GREATER, 1, ~0);  //draw if ==0
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); //set to 1

			glColor4f(0,0,0,.5);
		}
		else //Hgl::GetMode(HGL_SHADOW) == HGL_SHADOW_BLACK
		{
			Hgl::SetColor(Black);
		}
	}
}
void Hgl::Disable(Enum variable)
{
	GetCapEnable(variable) = false;
	if (variable == Hgl::SHADOW)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		Hgl::SetColor(White);
	}
}
bool Hgl::IsEnabled(Enum variable)
{
	return GetCapEnable(variable);
}
bool &Hgl::GetCapEnable(Enum variable)
{
	static bool invalid = false;
	switch (variable)
	{
	case Hgl::SHADOW :
		return curr->Shadow;
	default :
		throw HException("Hgl::GetCapEnable tried to access invalid mode");
	}
	return invalid;
}

//gl_shadows
void Hgl::SetShadows(int value)
{
	if(value<0 || value>2)
		throw HException(string("invalid value for gl_shadows: ")+to_string(value));
	curr->gl_shadows = value;
}
int Hgl::GetShadows() {return curr->gl_shadows;}

//gl_texturemode
void Hgl::SetTextureMode(Hgl::Enum value)
{
	if(!(value==Hgl::LINEAR_MIPMAP_LINEAR || value==Hgl::LINEAR || value==Hgl::NEAREST || Hgl::NONE))
		throw HException("invalid value for gl_texturemode");
	curr->gl_texturemode = value;
	Texture::SetRenderMode(value);
}
Hgl::Enum Hgl::GetTextureMode() {return curr->gl_texturemode;}

//gl_vertex_arrays
void Hgl::SetVertexArrays(bool value)
{
	//vertex arrays are part of the opengl 1.1 spec
	//if (value && !(GLExtentions.IsIn("GL_EXT_vertex_array") || GLExtentions.IsIn("GL_SGI_vertex_array")))
	//	throw HException("vertex arrays not available");
	curr->gl_vertex_arrays = value;
}
bool Hgl::GetVertexArrays() {return curr->gl_vertex_arrays;}

//gl_ext_compiled_vertex_array
void Hgl::SetExtCompiledVertexArrays(bool value)
{
	if (value && (curr->glLockArrays==NULL))
		throw HException("compiled vertex arrays unavailable");
	curr->gl_ext_compiled_vertex_array = value;
}
bool Hgl::GetExtCompiledVertexArrays() {return curr->gl_ext_compiled_vertex_array;}

//gl_finish
void Hgl::SetFinish(bool value) {curr->gl_finish = value;}
bool Hgl::GetFinish() {return curr->gl_finish;}

//conversions of Hgl::Enum's to text and vice versa
struct HglEnumDesc
{
	Hgl::Enum val;
	const char* name;
};

static HglEnumDesc EnumMapValues[] =
{
	{Hgl::INVALID_ENUM,			"GL_INVALID_ENUM"},
	{Hgl::NONE,					"GL_NONE"},
	{Hgl::LINEAR_MIPMAP_LINEAR,	"GL_LINEAR_MIPMAP_LINEAR"},
	{Hgl::LINEAR,				"GL_LINEAR"},
	{Hgl::NEAREST,				"GL_NEAREST"},
	{Hgl::SHADOW,				"HGL_SHADOW"}
};

const char* ToStr(Hgl::Enum val)
{
	for (unsigned int i=0 ; i<ARRAY_SIZE(EnumMapValues) ; i++)
	{
		if (val==EnumMapValues[i].val)
			return EnumMapValues[i].name;
	}
	return "GL_INVALID_ENUM";
}

Hgl::Enum ToHglEnum(const char* str)
{
	for (unsigned int i=0 ; i<ARRAY_SIZE(EnumMapValues) ; i++)
	{
		if (strcmp(str, EnumMapValues[i].name)==0)
			return EnumMapValues[i].val;
	}
	return Hgl::INVALID_ENUM;
}

istream& operator>> (istream &in, Hgl::Enum &A)
{
	string tmp;
	getline(in,tmp,'\0');
	A = ToHglEnum(tmp.c_str());
	return in;
}
ostream& operator<< (ostream &out, const Hgl::Enum &A)
{
	return out << ToStr(A);
}
