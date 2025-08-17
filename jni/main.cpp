#include <android/native_activity.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"Triangle",__VA_ARGS__)

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;
static GLuint program;

static const char *vshaderSrc =
    "attribute vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

static const char *fshaderSrc =
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(1.0,0.0,0.0,1.0);\n"
    "}\n";

GLuint loadShader(GLenum type, const char *src) {
	GLuint s = glCreateShader(type);
	glShaderSource(s, 1, &src, NULL);
	glCompileShader(s);
	return s;
}

void initGL(EGLNativeWindowType window) {
	EGLint attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	                     EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
	                     EGL_RED_SIZE, 8, EGL_NONE };
	EGLConfig config;
	EGLint numConfigs;
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	EGLint format;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	surface = eglCreateWindowSurface(display, config, window, NULL);
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttribs);
	eglMakeCurrent(display, surface, surface, context);

	GLuint vshader = loadShader(GL_VERTEX_SHADER, vshaderSrc);
	GLuint fshader = loadShader(GL_FRAGMENT_SHADER, fshaderSrc);
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	glUseProgram(program);
}

void drawFrame() {
	GLfloat verts[] = { 0.0f, 0.5f, 0.0f,
	                    -0.5f, -0.5f, 0.0f,
	                    0.5f, -0.5f, 0.0f };
	GLint posAttr = glGetAttribLocation(program, "vPosition");
	glViewport(0, 0, 1080, 1920);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray(posAttr);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	eglSwapBuffers(display, surface);
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
	if (cmd == APP_CMD_INIT_WINDOW && app->window != NULL) {
		initGL(app->window);
	}
}

void android_main(struct android_app* app) {
	app->onAppCmd = handle_cmd;
	int events;
	struct android_poll_source* source;
	while (1) {
		while (ALooper_pollAll(0, NULL, &events, (void**)&source) >= 0) {
			if (source) source->process(app, source);
			if (app->destroyRequested) return;
		}
		if (app->window) drawFrame();
	}
}

