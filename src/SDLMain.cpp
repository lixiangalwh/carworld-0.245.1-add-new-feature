
#include "H_Standard.h"

#include "H_Keys.h"
#include "H_Main.h"

#include "CarWorldClient.h"
#include "SDLJoystick.h"

#include <ctype.h>
#include <iostream>
#include <map>

#include <stdio.h>

#include <sys/timeb.h>
#include <time.h>

#include <SDL.h>

#include "CWMixerManager.h"
#include "CWBeeper.h"
#include "OFFObjectPool.h"

#include <boost/program_options.hpp>
#include "CommandOption.h"

HWindow::~HWindow() {}
HJoystick::~HJoystick(){}

class HSDLWindow : public HWindow
{
public:
	HSDLWindow();
	virtual ~HSDLWindow();
	virtual void SetAttrib(int width, int height, bool fullscreen);
	bool IsPressed(SDLKey k);
	const char* GetKeyboardDescription();
	HJoystick* GetJoystick();
	virtual void MakeCurrent();
	virtual void SwapBuffers();
public:
};

class HAppData
{
public:
	HAppData();
public:
};

HAppData::HAppData() {}

void HErrorExit(const char *E)
{
	cout << E << endl << flush;
	exit(1);
}

//CLASS HSDLWindow
HSDLWindow::HSDLWindow()
{}

HSDLWindow::~HSDLWindow()
{
}

const char *HSDLWindow::GetKeyboardDescription()
{
	return "SDL keyboard";
}

bool HSDLWindow::IsPressed(SDLKey k)
{
	Uint8* keys = SDL_GetKeyState(NULL);
	if (keys==NULL)
		return false;
	return keys[k]!=0;
}

HJoystick* HSDLWindow::GetJoystick()
{
	return new SDLJoystick(0);
}

void HSDLWindow::MakeCurrent()
{
//BUGBUG do we need this function for SDL?
	//doing this simple test is a _lot_ faster...
	/*static HSDLWindow* current = NULL;
	if (current!=this)
		wglMakeCurrent(hDC, hRC);
	current = this;*/
}

void HSDLWindow::SwapBuffers()
{
	SDL_GL_SwapBuffers();
}

void HSDLWindow::SetAttrib(int width, int height, bool fullscreen)
{
	m_width = width;
	m_height = height;
	m_fullscreen = fullscreen;
}

//default implementations for the application functions:
//CLASS HApplication
HApplication::HApplication() : m_data(new HAppData) {}
HApplication::~HApplication() {delete m_data;}
const char* HApplication::name() {return "generic HApplication";}
void HApplication::on_idle(unsigned int elapsed_time) {}

int HApplication::TimeRefreshRate()
{
	return 1000;
}

//CLASS HglApplication
HglApplication::HglApplication() : m_window(new HSDLWindow()) {}
HglApplication::~HglApplication() {delete m_window;}
void HglApplication::draw_init() {}
void HglApplication::key_down(SDLKey AHKey, char c) {}
void HglApplication::resize(unsigned int width, unsigned int height) {}
void HglApplication::draw() {}


//START Screen managment functions
unsigned int HglApplication::GetScreenMode()
{
//BUG implement this function
	cout << "WARNING: this function is not yet implemented." << endl;
	return 0;
}

bool HglApplication::SetScreenMode(unsigned int ScreenMode, ostream &out)
{
//BUG implement this function
	cout << "WARNING: this function is not yet implemented." << endl;
	return false;
}

void HglApplication::PrintScreenMode(unsigned int ScreenMode, ostream &out)
{
//BUG implement this function
	cout << "WARNING: this function is not yet implemented." << endl;
}

void HglApplication::PrintAllScreenModes(ostream &out)
{
//BUG implement this function
	cout << "WARNING: this function is not yet implemented." << endl;
}

const char* ConfigurationFileName()
{
	return "carworld.cfg";
}

int find(int argc, char **argv, const char *v)
{
	int i=0;
	while (i<argc && strcmp(argv[i],v)) i++;
	return i;
}

extern ofstream herr;

CarWorldClient* pCWC=NULL;

MixerManagerSingleton mixer;
//char g_in_database_name[1000];
int main(int argc, char *argv[])
{
	streambuf* cout_streambuf = cout.rdbuf();
	HglApplication* app = NULL;
	try
	{
		bool full_screen=false;
		bool not_save=false;

		// init command line options
		namespace po = boost::program_options;
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h","produce help message")
			("fullscreen,f","set fullscreen")
			("discard,n","discard the record data")
			("in-db,i",po::value<string>(&(CommandOption::Option().InDatabaseFile)),"input database.")
			("out-db,o",po::value<string>(&(CommandOption::Option().OutDatabaseFile)),"output database.")
			("in-dir,I",po::value<string>(&(CommandOption::Option().InDir)),"input search directory")
			("out-dir,O",po::value<string>(&(CommandOption::Option().OutDir)),"output directory")
			;
		po::variables_map vm;

		try
		{

			po::store(po::parse_command_line(argc,argv,desc),vm);
			po::notify(vm);
		}
		catch(po::error& e)
		{
			cout<<"error: "<<e.what()<<endl;
			cout<<desc<<"\n";
			return 1;
		}

		if(vm.count("help")){cout<<desc<<"\n";return 1;}
		else if(vm.count("fullscreen")){full_screen=true;}
		else if(vm.count("discard")){not_save=true;}

		CommandOption::Option().CheckOptions();// check the options
		
		HglApplication* app = pCWC = new CarWorldClient(full_screen,not_save);

		{
			SDL_version CompileVer;
			SDL_VERSION(&CompileVer);
			cout << "SDL compile time version: " << (int)CompileVer.major << "." << (int)CompileVer.minor << "." << (int)CompileVer.patch << endl;
		}

		{
			const SDL_version* DynamicVer = SDL_Linked_Version();
			cout << "SDL run time version: " << (int)DynamicVer->major << "." << (int)DynamicVer->minor << "." << (int)DynamicVer->patch << endl;
		}

		{
			cout << ((SDL_BYTEORDER==SDL_LIL_ENDIAN) ? "little endian" : "big endian") << endl;
		}

		Uint32 flags = SDL_OPENGL;
		if (full_screen)
			flags |= SDL_FULLSCREEN;
		else
			flags |= SDL_RESIZABLE;
		int bpp = 0; //the current display color depth
		int width = app->m_window->m_width;
		int height = app->m_window->m_height;
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER);
		SDL_EnableUNICODE(1);
		SDL_Surface* screen = SDL_SetVideoMode(width, height, bpp, flags);
		
		{
			char namebuf[256];
			SDL_VideoDriverName(namebuf, sizeof(namebuf));
			cout << "SDL video driver name: " << namebuf << endl;
		}

		if (screen==NULL)
		{
			cout << "ERROR: Couldn't set " << width << "x" << height << " GL video mode: " << SDL_GetError() << endl;
			SDL_Quit();
			return 2;
		}
		SDL_WM_SetCaption(app->name(), app->name());
		
		// init SDL_Mixer
		if(GetMixer().Init()==-1)
		{
			cout << "ERROR: Couldn't initiate sdl_mixer "<< endl;
			cout << GetMixer().GetError()<<endl;
			return 3;
		}

	//do OpenGL init
		app->draw_init();
		app->resize(screen->w, screen->h);

		//load the chunks
		AudioPlayer::shared_audio()->load_from_db();
		AudioPlayer::shared_audio()->get_sound("Background")->play_n_times(-1);

		
		bool done = false;
		Uint32 CurrentTime = SDL_GetTicks();
		while (!done)
		{
			//SDL_PumpEvents();
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch(event.type)
				{
				case SDL_KEYDOWN:
					{
						if (event.key.keysym.sym==SDLK_ESCAPE)
							done = true;
						else
						{
							char a = '\0';
							if (event.key.keysym.unicode<0x80 && event.key.keysym.unicode>0)
								a = (char)event.key.keysym.unicode;
							app->key_down(event.key.keysym.sym, a);
						}
					}
					break;
				case SDL_VIDEORESIZE:
					{
	//BUG if i call SDL_SetVideoMode() on win32 i lose all textures and lists...
#ifndef WIN32
						SDL_SetVideoMode(event.resize.w, event.resize.h, bpp, flags);
#endif //WIN32
						app->resize(event.resize.w, event.resize.h);
					}
					break;
				case SDL_MOUSEMOTION:
					{
						app->mouse_motion(event.motion);
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					{
						if(event.button.button==SDL_BUTTON_WHEELDOWN || event.button.button==SDL_BUTTON_WHEELUP)
						{
							app->mouse_wheel(event.button);
						}
						break;
					}
				case SDL_QUIT:
					done = true;
					break;
				}
			}
			Uint32 NewTime = SDL_GetTicks();
			app->on_idle(NewTime-CurrentTime);
			CurrentTime = NewTime;
			app->draw();
		}
		OFFObjectPool::release();
		delete app;
		app = pCWC = NULL;
		cout.rdbuf(cout_streambuf);
		AudioPlayer::release_audio();
		GetMixer().Quit();
		SDL_Quit();
		return 0;
	}
	catch (const HException &E)
	{
	//resor cout to normal
		cout.rdbuf(cout_streambuf);

	//write the error message in log file and standard out
		herr << E.what() << endl << flush;
		cout << E.what() << endl << flush;

	//free resources
	//we do this last in case it crashes...
		delete app;
		return 1;
	}
}
