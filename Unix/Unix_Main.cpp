#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "../GL/FunctionDefs.h"
#include "../ParticleSimulation.cpp"

#include "X11/XKBlib.h"

struct Unix_Display_State_Data
{
  Display* DisplayServer = nullptr;
  Window RootWindow = 0;
  Window MainWindow = 0;
  XVisualInfo* VisualInfo = nullptr;

  GLXContext GLContext;

  int DisplayServerFD = 0;
  bool ShouldCloseDisplay = false;
};

struct GLRenderAsset
{
  GLuint VAO;
  GLuint ShaderProgram;
  GLuint ElementCount; // Number of elements in VAO's internal EBO. 
};

struct ParticleRenderCommand
{
  Matrix4x4 WorldTransform; // 4x4 World Transform Matrix
  ColorRGB Color;
  float Radius;
};



Unix_Display_State_Data UnixDisplayState;

GLRenderAsset DebugParticle;

ParticleRenderCommand RenderCommands[512];
int ParticleRenderCommandCount = 0;

Matrix4x4 ViewportMatrix;

// SIMULATION COMMANDS

void ExitApplication()
{
  UnixDisplayState.ShouldCloseDisplay = true;
}

void AddParticleRenderCommand(Matrix4x4 TransformMatrix, ColorRGB Color, float Radius)
{
  RenderCommands[ParticleRenderCommandCount].WorldTransform = TransformMatrix;
  RenderCommands[ParticleRenderCommandCount].Color = Color;
  RenderCommands[ParticleRenderCommandCount].Radius = Radius;
  
  ParticleRenderCommandCount++;
}

void SetViewportMatrix(Matrix4x4 NewViewMatrix)
{
  ViewportMatrix = NewViewMatrix;
}

void OpenGL_DrawParticles()
{ 
  glClearColor(0.1, 0.3, 0.6, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glBindVertexArray(DebugParticle.VAO);
  glUseProgram(DebugParticle.ShaderProgram);


  GLuint VertexBaseColorLocation = glGetUniformLocation(DebugParticle.ShaderProgram, "BaseVertexColor");
  GLuint ViewspaceMatrixLocation = glGetUniformLocation(DebugParticle.ShaderProgram, "ViewspaceMatrix");
  GLuint WidthToHeightRatioLocation = glGetUniformLocation(DebugParticle.ShaderProgram, "WidthToHeightRatio");
  glUniform1f(WidthToHeightRatioLocation, 1920.f / 1080.f);
      
  
  for(int ParticleRenderCommandIndex = 0; ParticleRenderCommandIndex < ParticleRenderCommandCount; ParticleRenderCommandIndex++)
    {
      ColorRGB ParticleColor = RenderCommands[ParticleRenderCommandIndex].Color;
      glUniform4f(VertexBaseColorLocation, ParticleColor.r, ParticleColor.g, ParticleColor.b, 1.f);

      // Compute View Matrix for this Particle.

      Matrix4x4 ViewspaceMatrix = RenderCommands[ParticleRenderCommandIndex].WorldTransform;

      // Transform World Matrix into Viewspace Matrix by applying Viewport matrix.
      ViewspaceMatrix.AddTranslation(-ViewportMatrix.GetTranslation());
      
      const GLfloat* ViewspaceMatrixData = reinterpret_cast<const GLfloat*>(&ViewspaceMatrix);
      glUniformMatrix4fv(ViewspaceMatrixLocation, 1, true, ViewspaceMatrixData);

      glDrawElements(GL_TRIANGLES, DebugParticle.ElementCount, GL_UNSIGNED_INT, 0);
    }
  
  ParticleRenderCommandCount = 0;
  glBindVertexArray(DebugParticle.VAO);
}

// Reads a file, allocates enough memory to hold it in its entirety and returns pointer to the allocated memory containing the file.
char* ReadFileIntoMemory(const char* FileName)
{
  static char READ_MEMORY[4096];

  memset(READ_MEMORY, 0, 4096);

  char* Memory = nullptr;
  int FD = open(FileName, O_RDONLY);

  if (FD < 0)
    {
      printf("ERROR - Couldn't read file '%s' !\n", FileName);
      return nullptr;
    }
  
  ssize_t ReadBytes = read(FD, READ_MEMORY, 4096);

  Memory = static_cast<char*>(malloc(ReadBytes + 1)); // Size of entire file + Null terminator
  memcpy(Memory, READ_MEMORY, ReadBytes);
  Memory[ReadBytes] = '\0';
  
  close(FD);

  return Memory;
}

// Loads a Mesh and an entire associated Shader Program from files into the passed GLRenderAsset.
// The data becomes accessible and usable through OpenGL object handles (therefore the data is loaded in video memory directly).
void LoadRenderAsset(const char* MeshDataFile, const char* VertexShaderFile, const char* FragmentShaderFile,
		     GLRenderAsset& RenderAsset)
{
  // Compile Shaders & Create Shader Program
  {
    char* VertexShader = ReadFileIntoMemory(VertexShaderFile);
    char* FragmentShader = ReadFileIntoMemory(FragmentShaderFile);

    printf("\nLOADED VERTEX SHADER:\n%s\n", VertexShader);
    printf("\nLOADED FRAGMENT SHADER:\n%s\n", FragmentShader);
  
    int VertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    int FragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    {
      const GLchar* VertexShaderSource = static_cast<GLchar*>(VertexShader);
      const GLchar* FragmentShaderSource = static_cast<GLchar*>(FragmentShader);
      
      glShaderSource(VertexShaderObject, 1, &VertexShaderSource, NULL);
      glShaderSource(FragmentShaderObject, 1, &FragmentShaderSource, NULL);
    }
    
    free(VertexShader);
    free(FragmentShader);
    
    GLuint SuccessCode;
    GLchar CompilationLog[512];
    
    glCompileShader(VertexShaderObject);

    glGetShaderiv(VertexShaderObject, GL_COMPILE_STATUS, &SuccessCode);
    if (!SuccessCode)
      {
	printf("\nFailed to compile Vertex Shader.\n");
	glGetShaderInfoLog(VertexShaderObject, 512, NULL, CompilationLog);
	printf("\nLogs:\n%s\n", CompilationLog);
      }
    
    glCompileShader(FragmentShaderObject);

    glGetShaderiv(FragmentShaderObject, GL_COMPILE_STATUS, &SuccessCode);
    if (!SuccessCode)
      {
	printf("Failed to compile Fragment Shader.\n");
	glGetShaderInfoLog(FragmentShaderObject, 512, NULL, CompilationLog);
	printf("\nLogs:\n%s\n", CompilationLog);
      }

    RenderAsset.ShaderProgram = glCreateProgram();
    glAttachShader(RenderAsset.ShaderProgram, VertexShaderObject);
    glAttachShader(RenderAsset.ShaderProgram, FragmentShaderObject);
    glLinkProgram(RenderAsset.ShaderProgram);

    glDeleteShader(VertexShaderObject);
    glDeleteShader(FragmentShaderObject);
  }

  // Load Mesh create VAO.
  {
      float Vertex[] =
	{
	  0.f, -0.5f, -0.5f,
	  0.f, 0.5f, -0.5f,
	  0.0f, 0.5f, 0.5f,
	  0.0f, -0.5f, 0.5f
	};

      unsigned int Elements[] =
	{
	  2, 1, 0,
	  3, 2, 0
	};
      
      glGenVertexArrays(1, &RenderAsset.VAO);
      glBindVertexArray(RenderAsset.VAO);

      GLuint VBO;
      glGenBuffers(1, &VBO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      
      glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), Vertex, GL_STATIC_DRAW);
      
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
      glEnableVertexAttribArray(0);
      
      GLuint EBO;
      glGenBuffers(1, &EBO);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);
      
      RenderAsset.ElementCount = sizeof(Elements) / sizeof(unsigned int);
      
      glBindVertexArray(NULL);
  }
}

bool InitializeDisplayState(const char* MainWindowName, int WindowWidth, int WindowHeight)
{
  Unix_Display_State_Data& dp = UnixDisplayState;

  printf("\nInitializing Display State...\n");

  // Initialize connection to X Display Server.
  dp.DisplayServer = XOpenDisplay(NULL);
  if (dp.DisplayServer == nullptr)
    {
      printf("\n\t ERROR INITIALIZING DISPLAY STATE: Cannot Connect to X Server.\n");
      return false;
    }

  // Create Root Window.
  dp.RootWindow = DefaultRootWindow(dp.DisplayServer);
  
  // Use default Display Attributes to pick X Visual Info object.
  GLint DisplayAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  dp.VisualInfo = glXChooseVisual(dp.DisplayServer, 0, DisplayAttributes);

  if (dp.VisualInfo == NULL)
    {
      printf("\n\t ERROR INITIALIZING DISPLAY STATE: No appropriate Visual Info found !\n");
      return false;
    }  

  // Create and assign Color Map from Visual through a Window Attributes Data structure.
  Colormap ColorMap = XCreateColormap(dp.DisplayServer, dp.RootWindow, dp.VisualInfo->visual, AllocNone);
  XSetWindowAttributes SetWindowAttributesData = {0};
  SetWindowAttributesData.colormap = ColorMap;

  // Handle Exposure and Key Press events.
  SetWindowAttributesData.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;

  // Create Main Window the simulation will be drawn onto.
  dp.MainWindow = XCreateWindow(dp.DisplayServer, dp.RootWindow, 0, 0, WindowWidth, WindowHeight, 0, dp.VisualInfo->depth, InputOutput, dp.VisualInfo->visual, CWColormap | CWEventMask, &SetWindowAttributesData);
  XMapWindow(dp.DisplayServer, dp.MainWindow);
  XStoreName(dp.DisplayServer, dp.MainWindow, MainWindowName);

  // Load OpenGL Function Definitions that have to be loaded dynamically.
  LoadOpenGLFunctions();

  // Create OpenGL Context linked to Display Server and make it the Main Window's current context.
  GLXContext GLContext = glXCreateContext(dp.DisplayServer, dp.VisualInfo, NULL, GL_TRUE);
  glXMakeCurrent(dp.DisplayServer, dp.MainWindow, GLContext);

  // Config
  XkbSetDetectableAutoRepeat(dp.DisplayServer, false, nullptr);
  XAutoRepeatOff(dp.DisplayServer);
  
  printf("\nDisplay State initialized.\n");
  return true;
}



int main(int argc, char* argv[])
{
  InitializeDisplayState("Particles Simulation", 1920, 1080);

  LoadRenderAsset("./Mesh_DebugParticle.mesh", "./VertexShader.shader", "./FragmentShader.shader", DebugParticle);
  
  glEnable(GL_DEPTH_TEST);

  printf("Program ready. Launching main loop.\n");

  SimulationContext Context;
  Context.SendParticleRenderCommand = AddParticleRenderCommand;
  Context.SetViewportMatrix = SetViewportMatrix;
  Context.SendExitApplicationCommand = ExitApplication;
  
  UnixDisplayState.DisplayServerFD = ConnectionNumber(UnixDisplayState.DisplayServer);
  
  fd_set WindowEventPollingFDSet;
  timeval WindowEventPollingTimeval = {0, 0};
  int FrameStartClock, FrameEndClock;
  float LastFrameTimeDeltaSeconds = 0.f;
  while(!UnixDisplayState.ShouldCloseDisplay)
    {
      // Event Handling
      FD_ZERO(&WindowEventPollingFDSet);
      FD_SET(UnixDisplayState.DisplayServerFD, &WindowEventPollingFDSet);

      int ReadyFDs = select(UnixDisplayState.DisplayServerFD + 1, &WindowEventPollingFDSet, NULL, NULL, &WindowEventPollingTimeval); 
      if (ReadyFDs >= 0)
	{
	  XEvent NextEvent;
	  while (XPending(UnixDisplayState.DisplayServer))
	    {
	      XNextEvent(UnixDisplayState.DisplayServer, &NextEvent);

	      if (NextEvent.type == Expose)
		{
		  XWindowAttributes WindowAttributes = {0};
		  XGetWindowAttributes(UnixDisplayState.DisplayServer, UnixDisplayState.MainWindow, &WindowAttributes);
		  glViewport(0, 0, WindowAttributes.width, WindowAttributes.height);
		}
	      else if (NextEvent.type == KeyPress)
		{
		  XKeyEvent KeyEvent = NextEvent.xkey;
		  KeySym PressedKey = XKeycodeToKeysym(UnixDisplayState.DisplayServer, KeyEvent.keycode, NULL);
		  // Handle Alphanumeric key events.
		  if (PressedKey >= 'a' && PressedKey <= 'z')
		    {
		      Context.InputStates[PressedKey - 'a'] = SimulationInputState::PRESSED;
		    }

		  // Handle Special key events
		  if (NextEvent.xkey.keycode == 9)
		    {
		      Context.InputStates[static_cast<int>(SimulationInputKey::ESCAPE)] = SimulationInputState::PRESSED;
		    }
		}
	      else if (NextEvent.type == KeyRelease)
		{
		  XKeyEvent KeyEvent = NextEvent.xkey;
		  KeySym ReleasedKey = XKeycodeToKeysym(UnixDisplayState.DisplayServer, KeyEvent.keycode, NULL);

		  // Handle Alphanumeric key events.
		  if (ReleasedKey >= 'a' && ReleasedKey <= 'z')
		    {
		      Context.InputStates[ReleasedKey - 'a'] = SimulationInputState::RELEASED;
		    }
		  if (NextEvent.xkey.keycode == 9)
		    {
		      Context.InputStates[static_cast<int>(SimulationInputKey::ESCAPE)] = SimulationInputState::RELEASED;
		    }
		}
	    }
	}

      // Frame
      FrameStartClock = clock();
      RunSimulation(Context, LastFrameTimeDeltaSeconds);
      OpenGL_DrawParticles();
      glXSwapBuffers(UnixDisplayState.DisplayServer, UnixDisplayState.MainWindow);
      FrameEndClock = clock();
      
      LastFrameTimeDeltaSeconds = (FrameEndClock - FrameStartClock) / static_cast<float>(CLOCKS_PER_SEC);

      // "Advance" all inputs (Pressed -> Held, Released -> None).
      for(int InputKeyIndex = 0; InputKeyIndex < static_cast<int>(SimulationInputKey::KEY_COUNT); InputKeyIndex++)
	{
	  switch(Context.InputStates[InputKeyIndex])
	    {
	    case(SimulationInputState::PRESSED):
	      Context.InputStates[InputKeyIndex] = SimulationInputState::HELD;
	      break;
	    case(SimulationInputState::RELEASED):
	      Context.InputStates[InputKeyIndex] = SimulationInputState::NONE;
	      break;
	    default:
	      break;
	    }
	}
    }

  glXMakeCurrent(UnixDisplayState.DisplayServer, None, NULL);
  glXDestroyContext(UnixDisplayState.DisplayServer, UnixDisplayState.GLContext);
  XDestroyWindow(UnixDisplayState.DisplayServer, UnixDisplayState.MainWindow);
    
  printf("Closing Display...\n");
  XCloseDisplay(UnixDisplayState.DisplayServer);
  return 0;
}
