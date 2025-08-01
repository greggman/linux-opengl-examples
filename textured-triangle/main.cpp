// build: g++ main.cpp -o main -lX11 -lGL
// run: DISPLAY=:0 ./main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

//-----------------------------------------------------------------------------------
// OpenGL Function Pointers
//-----------------------------------------------------------------------------------

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;

void init_gl_functions() {
    glCreateShader = (PFNGLCREATESHADERPROC)glXGetProcAddress((const GLubyte*)"glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)glXGetProcAddress((const GLubyte*)"glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)glXGetProcAddress((const GLubyte*)"glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)glXGetProcAddress((const GLubyte*)"glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glXGetProcAddress((const GLubyte*)"glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)glXGetProcAddress((const GLubyte*)"glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glXGetProcAddress((const GLubyte*)"glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glXGetProcAddress((const GLubyte*)"glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glUseProgram");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte*)"glBindVertexArray");
    glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte*)"glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress((const GLubyte*)"glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress((const GLubyte*)"glVertexAttribPointer");
    glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)glXGetProcAddress((const GLubyte*)"glBindAttribLocation");
}

//-----------------------------------------------------------------------------------
// OpenGL Helpers
//-----------------------------------------------------------------------------------

void checkError(const char *msg) {
  GLenum err = glGetError();
  if (err) {
      printf("Err %s: %x\n", msg, err);
      exit(1);
  }
}

GLint compileShader(GLenum type, const char* src)
{
    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        GLchar *log = (GLchar*)malloc(len);
        glGetShaderInfoLog(shader, len, nullptr, log);
        printf("Failed to compile!: %s\n", log);
        free(log);
    }
    checkError("compiling");
    return shader;
}

void linkProgram(GLuint program) {
  glLinkProgram(program);
  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
      GLint len;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
      GLchar *log = (GLchar*)malloc(len);
      glGetProgramInfoLog(program, len, &len, log);
      printf("Failed to link!: %s\n", log);
      free(log);
      exit(1);
  }
}

//-----------------------------------------------------------------------------------
// main
//-----------------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
    // 1. Open a connection to the X server
    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        printf("Cannot connect to X server\n");
        return 1;
    }

    // 2. Choose a suitable visual
    static int visual_attribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };
    XVisualInfo *vi = glXChooseVisual(dpy, DefaultScreen(dpy), visual_attribs);
    if (vi == NULL) {
        printf("No appropriate visual found\n");
        return 1;
    }

    // 3. Create a window
    Colormap cmap = XCreateColormap(dpy, DefaultRootWindow(dpy), vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;
    Window win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, 256, 256, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Textured Triangle");

    // 4. Create an OpenGL context and make it current
    GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    // Initialize GL functions
    init_gl_functions();

    // 5. Compile and link shaders
    GLuint program = glCreateProgram();
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER,
        R"RAW(#version 460
        in vec2 p;
        in vec2 uv;
        out vec2 v_uv;
        void main() {
          gl_Position = vec4(p, 0, 1);
          v_uv = uv;
        }
        )RAW");

        GLuint fs = compileShader(GL_FRAGMENT_SHADER,
        R"RAW(#version 460
         uniform sampler2D u_tex;
         in vec2 v_uv;
         out vec4 fragColor;
         void main() {
           fragColor = texture(u_tex, v_uv);
         }
        )RAW");

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glBindAttribLocation(program, 0, "p");
        glBindAttribLocation(program, 1, "uv");
        linkProgram(program);
    }
    checkError("programs");

    // 6. Create texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte pixels[] = {
        255, 0, 0, 255,    0, 255, 0, 255,
        0, 0, 255, 255,    255, 255, 0, 255
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    checkError("texture");

    // 7. Create vertex array
    GLuint va;
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    GLuint buf;
    glGenBuffers(1, &buf);
    static const float data[] = {
        // positions     // uvs
        -0.5f, -0.5f,    0.0f, 0.0f,
         0.5f, -0.5f,    1.0f, 0.0f,
         0.0f,  0.5f,    0.5f, 1.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
    checkError("va");

    // 8. Main loop
    while (1) {
        XEvent xev;
        XNextEvent(dpy, &xev);
        if (xev.type == Expose) {
            glViewport(0, 0, 256, 256);
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(program);
            glBindTexture(GL_TEXTURE_2D, tex);
            glBindVertexArray(va);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glXSwapBuffers(dpy, win);
        } else if (xev.type == KeyPress) {
            break;
        }
    }

    // 9. Cleanup
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}