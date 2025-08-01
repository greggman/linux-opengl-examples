// build: g++ main.cpp -o main -lX11 -lGL
// run: ./main

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
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

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
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindFramebuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glXGetProcAddress((const GLubyte*)"glFramebufferTexture2D");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glXGetProcAddress((const GLubyte*)"glCheckFramebufferStatus");
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

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(A[0]))
GLenum swizzles[][4] = {
  { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA },
  { GL_ONE, GL_ONE, GL_ONE, GL_ONE },
  { GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO },
  { GL_RED, GL_RED, GL_RED, GL_RED },
  { GL_GREEN, GL_GREEN, GL_GREEN, GL_GREEN },
  { GL_BLUE, GL_BLUE, GL_BLUE, GL_BLUE },
  { GL_ALPHA, GL_ALPHA, GL_ALPHA, GL_ALPHA },
  { GL_ALPHA, GL_BLUE, GL_GREEN, GL_RED },
};
GLenum compares[] = {
  GL_LESS,
  GL_GREATER,
};

const char* glEnumToString(GLenum cmp) {
  switch (cmp) {
    case GL_LESS: return "less";
    case GL_GREATER: return "greater";
    default: return "unknown enum!";
  }
}

const char* swizzleToString(GLenum sw) {
  switch (sw) {
    case GL_RED: return "r";
    case GL_GREEN: return "g";
    case GL_BLUE: return "b";
    case GL_ALPHA: return "a";
    case GL_ONE: return "1";
    case GL_ZERO: return "0";
    default: return "unknown!!!";
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
    Window win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, 100, 100, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "OpenGL Test");

    // 4. Create an OpenGL context and make it current
    GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    // Initialize GL functions
    init_gl_functions();

    printf("version : %s\n", glGetString(GL_VERSION));
    printf("vendor  : %s\n", glGetString(GL_VENDOR));
    printf("renderer: %s\n", glGetString(GL_RENDERER));
    printf("GLSL ver: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    checkError("setup");

    // 5. Compile and link shaders
    GLuint texProgram = glCreateProgram();
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER,
        R"RAW(#version 460
        in vec2 p;
        void main() {
          gl_Position = vec4(p, 0, 1);
        }
        )RAW");

        GLuint fs = compileShader(GL_FRAGMENT_SHADER,
        R"RAW(#version 460
         #extension GL_ARB_texture_gather : require
         uniform sampler2DShadow u_tex;
         out vec4 fragColor;
         void main() {
           fragColor = textureGather(u_tex, vec2(0.5), 0.5);
         }
        )RAW");

        glAttachShader(texProgram, vs);
        glAttachShader(texProgram, fs);
        glBindAttribLocation(texProgram, 0, "p");
        linkProgram(texProgram);
    }
    checkError("programs");

    // 6. Create depth texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 2, 2, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
    checkError("texture1");

    // 7. Create color texture and framebuffer to fill the depth texture
    GLuint tex2;
    glGenTextures(1, &tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);
    checkError("texture2");

    GLuint fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("Framebuffer incomplete\n");
      exit(1);
    }

    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < 4; ++i) {
        int x = i % 2;
        int y = i / 2;
        glViewport(x, y, 1, 1);
        glScissor(x, y, 1, 1);
        glClearDepth(i * 0.2 + 0.2);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkError("fill");

    // 8. Create vertex array for drawing a quad
    GLuint va;
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    GLuint buf;
    glGenBuffers(1, &buf);
    static const float quad[] = { -1,-1, 1,-1, -1,1, -1,1, 1,-1, 1,1 };
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    checkError("va");

    // 9. Create a framebuffer for reading results
    GLuint tex3;
    glGenTextures(1, &tex3);
    glBindTexture(GL_TEXTURE_2D, tex3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLuint fb2;
    glGenFramebuffers(1, &fb2);
    glBindFramebuffer(GL_FRAMEBUFFER, fb2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex3, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("Framebuffer2 incomplete\n");
      exit(1);
    }
    glViewport(0, 0, 1, 1);

    // 10. Run tests
    glUseProgram(texProgram);
    glBindTexture(GL_TEXTURE_2D, tex);
    for (int cmp = 0; cmp < ARRAY_SIZE(compares); ++cmp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, compares[cmp]);
        printf("compare: %s\n", glEnumToString(compares[cmp]));

        for (int sw = 0; sw < ARRAY_SIZE(swizzles); ++sw) {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzles[sw][0]);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzles[sw][1]);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzles[sw][2]);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, swizzles[sw][3]);

          glDrawArrays(GL_TRIANGLES, 0, 6);

          std::vector<uint8_t> pixel(4);
          glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel[0]);
          checkError("read");

          printf("%g %g %g %g : swizzle: %s %s %s %s\n",
                float(pixel[0]) / 255.0f, float(pixel[1]) / 255.0f, float(pixel[2]) / 255.0f, float(pixel[3]) / 255.0f,
                swizzleToString(swizzles[sw][0]),
                swizzleToString(swizzles[sw][1]),
                swizzleToString(swizzles[sw][2]),
                swizzleToString(swizzles[sw][3])
                );
        }
    }

    // 11. Cleanup
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}