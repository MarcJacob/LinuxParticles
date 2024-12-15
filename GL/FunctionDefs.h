
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glu.h>

#define LOAD_GL_FUNC(GL_FUNC_TYPE_NAME, GlFunctionName) reinterpret_cast<GL_FUNC_TYPE_NAME>(glXGetProcAddress(reinterpret_cast<const GLubyte*>(GlFunctionName)))

typedef void (*GL_GEN_BUFFERS_FUNC)(GLsizei n, GLuint* buffers);
GL_GEN_BUFFERS_FUNC glGenBuffers;

typedef void (*GL_BIND_BUFFER_FUNC)(GLenum target, GLuint buffer);
GL_BIND_BUFFER_FUNC glBindBuffer;

typedef void (*GL_BUFFER_DATA_FUNC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
GL_BUFFER_DATA_FUNC glBufferData;

typedef void (*GL_GEN_VERTEX_ARRAYS_FUNC)(GLsizei n, GLuint* vaos);
GL_GEN_VERTEX_ARRAYS_FUNC glGenVertexArrays;

typedef void (*GL_BIND_VERTEX_ARRAY_FUNC)(GLuint vao);
GL_BIND_VERTEX_ARRAY_FUNC glBindVertexArray;

typedef void (*GL_VERTEX_ATTRIB_POINTER_FUNC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* data);
GL_VERTEX_ATTRIB_POINTER_FUNC glVertexAttribPointer;

typedef void (*GL_ENABLE_VERTEX_ATTRIB_ARRAY_FUNC)(GLuint n);
GL_ENABLE_VERTEX_ATTRIB_ARRAY_FUNC glEnableVertexAttribArray;

typedef GLuint (*GL_CREATE_SHADER_FUNC)(GLenum shaderType);
GL_CREATE_SHADER_FUNC glCreateShader;

typedef void (*GL_SHADER_SOURCE_FUNC)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
GL_SHADER_SOURCE_FUNC glShaderSource;

typedef void (*GL_COMPILE_SHADER_FUNC)(GLuint shader);
GL_COMPILE_SHADER_FUNC glCompileShader;

typedef GLuint (*GL_CREATE_PROGRAM_FUNC)();
GL_CREATE_PROGRAM_FUNC glCreateProgram;

typedef void (*GL_ATTACH_SHADER_FUNC)(GLuint program, GLuint shader);
GL_ATTACH_SHADER_FUNC glAttachShader;

typedef void (*GL_LINK_PROGRAM_FUNC)(GLuint program);
GL_LINK_PROGRAM_FUNC glLinkProgram;

typedef void (*GL_USE_PROGRAM_FUNC)(GLuint program);
GL_USE_PROGRAM_FUNC glUseProgram;

typedef void (*GL_GET_SHADER_IV_FUNC)(GLuint shader, GLenum param, GLuint* success);
GL_GET_SHADER_IV_FUNC glGetShaderiv;

typedef void (*GL_GET_SHADER_INFO_LOG_FUNC)(GLuint shader, GLsizei maxSize, GLsizei* length, GLchar* buffer); 
GL_GET_SHADER_INFO_LOG_FUNC glGetShaderInfoLog;

typedef void (*GL_DELETE_SHADER_FUNC)(GLuint shader);
GL_DELETE_SHADER_FUNC glDeleteShader;

typedef GLuint (*GL_GET_UNIFORM_LOCATION_FUNC)(GLuint program, GLchar* uniformName);
GL_GET_UNIFORM_LOCATION_FUNC glGetUniformLocation;

typedef void (*GL_UNIFORM_4F_FUNC)(GLuint uniformLocation, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_UNIFORM_4F_FUNC glUniform4f;

typedef void (*GL_UNIFORM_1F_FUNC)(GLuint uniformLocation, GLfloat value);
GL_UNIFORM_1F_FUNC glUniform1f;

typedef void (*GL_UNIFORM_MATRIX_4FV_FUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_UNIFORM_MATRIX_4FV_FUNC glUniformMatrix4fv;

void LoadOpenGLFunctions()
{
  glGenBuffers = LOAD_GL_FUNC(GL_GEN_BUFFERS_FUNC, "glGenBuffers");
  glBindBuffer = LOAD_GL_FUNC(GL_BIND_BUFFER_FUNC, "glBindBuffer");
  glBufferData = LOAD_GL_FUNC(GL_BUFFER_DATA_FUNC, "glBufferData");

  glGenVertexArrays = LOAD_GL_FUNC(GL_GEN_VERTEX_ARRAYS_FUNC, "glGenVertexArrays");
  glBindVertexArray = LOAD_GL_FUNC(GL_BIND_VERTEX_ARRAY_FUNC, "glBindVertexArray");
  glVertexAttribPointer = LOAD_GL_FUNC(GL_VERTEX_ATTRIB_POINTER_FUNC, "glVertexAttribPointer");
  glEnableVertexAttribArray = LOAD_GL_FUNC(GL_ENABLE_VERTEX_ATTRIB_ARRAY_FUNC, "glEnableVertexAttribArray");

  glCreateShader = LOAD_GL_FUNC(GL_CREATE_SHADER_FUNC, "glCreateShader");
  glShaderSource = LOAD_GL_FUNC(GL_SHADER_SOURCE_FUNC, "glShaderSource");
  glCompileShader = LOAD_GL_FUNC(GL_COMPILE_SHADER_FUNC, "glCompileShader");
  glAttachShader = LOAD_GL_FUNC(GL_ATTACH_SHADER_FUNC, "glAttachShader");
  glGetShaderiv = LOAD_GL_FUNC(GL_GET_SHADER_IV_FUNC, "glGetShaderiv");
  glGetShaderInfoLog = LOAD_GL_FUNC(GL_GET_SHADER_INFO_LOG_FUNC, "glGetShaderInfoLog");
  glDeleteShader = LOAD_GL_FUNC(GL_DELETE_SHADER_FUNC, "glDeleteShader");
  
  glCreateProgram = LOAD_GL_FUNC(GL_CREATE_PROGRAM_FUNC, "glCreateProgram");
  glLinkProgram = LOAD_GL_FUNC(GL_LINK_PROGRAM_FUNC, "glLinkProgram");
  glUseProgram = LOAD_GL_FUNC(GL_USE_PROGRAM_FUNC, "glUseProgram");

  glGetUniformLocation = LOAD_GL_FUNC(GL_GET_UNIFORM_LOCATION_FUNC, "glGetUniformLocation");
  glUniform4f = LOAD_GL_FUNC(GL_UNIFORM_4F_FUNC, "glUniform4f");
  glUniform1f = LOAD_GL_FUNC(GL_UNIFORM_1F_FUNC, "glUniform1f");
  glUniformMatrix4fv = LOAD_GL_FUNC(GL_UNIFORM_MATRIX_4FV_FUNC, "glUniformMatrix4fv");
}


