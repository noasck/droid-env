#include <jni.h>
#include <android/native_activity.h>
#include <android/input.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <string>
#include <sstream>

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;
static int width, height;

static void drawText(float x, float y, const std::string &msg) {
	// Minimal placeholder: clear screen, change clear color by touch X/Y
	float r = x / width;
	float g = y / height;
	glClearColor(r, g, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	eglSwapBuffers(display, surface);
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		float x = AMotionEvent_getX(event, 0);
		float y = AMotionEvent_getY(event, 0);
		std::ostringstream ss;
		ss << "Touch: " << x << "," << y;
		drawText(x, y, ss.str());
		return 1;
	}
	return 0;
}

void android_main(struct android_app* app) {
	app->onInputEvent = handle_input;

	EGLint attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };
	EGLint numConfigs;
	EGLConfig config;
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	EGLint format;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(app->window, 0, 0, format);
	EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	surface = eglCreateWindowSurface(display, config, app->window, NULL);
	context = eglCreateContext(display, config, NULL, ctxAttribs);
	eglMakeCurrent(display, surface, surface, context);
	eglQuerySurface(display, surface, EGL_WIDTH, &width);
	eglQuerySurface(display, surface, EGL_HEIGHT, &height);

	while (1) {
		int events;
		struct android_poll_source* source;
		while (ALooper_pollAll(0, NULL, &events, (void**)&source) >= 0) {
			if (source) source->process(app, source);
			if (app->destroyRequested) return;
		}
	}
}

