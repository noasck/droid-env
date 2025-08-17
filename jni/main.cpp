#include <android/native_activity.h>
#include <android/log.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	ANativeActivity* activity;
	ANativeWindow* window;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int running;
	pthread_t thread;
	GLuint prog;
	GLuint vbo;
} App;

static const char* vs_src = "attribute vec2 aPos;\nvoid main(){gl_Position=vec4(aPos,0.0,1.0);}";
static const char* fs_src = "precision mediump float;\nvoid main(){gl_FragColor=vec4(1.0,0.6,0.1,1.0);}";

static GLuint compile(GLenum type, const char* src){
	GLuint s = glCreateShader(type);
	glShaderSource(s,1,&src,0);
	glCompileShader(s);
	return s;
}

static GLuint link(GLuint vs, GLuint fs){
	GLuint p = glCreateProgram();
	glAttachShader(p,vs);
	glAttachShader(p,fs);
	glBindAttribLocation(p,0,"aPos");
	glLinkProgram(p);
	glDeleteShader(vs);
	glDeleteShader(fs);
	return p;
}

static int init_gl(App* app){
	EGLint cfg_attr[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_DEPTH_SIZE,0,EGL_NONE};
	EGLConfig cfg; EGLint num;
	app->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(app->display==EGL_NO_DISPLAY) return 0;
	if(!eglInitialize(app->display,0,0)) return 0;
	if(!eglChooseConfig(app->display,cfg_attr,&cfg,1,&num)) return 0;
	EGLint format; eglGetConfigAttrib(app->display,cfg,EGL_NATIVE_VISUAL_ID,&format);
	ANativeWindow_setBuffersGeometry(app->window,0,0,format);
	app->surface = eglCreateWindowSurface(app->display,cfg,app->window,0);
	EGLint ctx_attr[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};
	app->context = eglCreateContext(app->display,cfg,EGL_NO_CONTEXT,ctx_attr);
	if(app->surface==EGL_NO_SURFACE||app->context==EGL_NO_CONTEXT) return 0;
	if(!eglMakeCurrent(app->display,app->surface,app->surface,app->context)) return 0;
	GLuint vs=compile(GL_VERTEX_SHADER,vs_src);
	GLuint fs=compile(GL_FRAGMENT_SHADER,fs_src);
	app->prog=link(vs,fs);
	GLfloat verts[]={0.0f,0.6f,-0.6f,-0.6f,0.6f,-0.6f};
	glGenBuffers(1,&app->vbo);
	glBindBuffer(GL_ARRAY_BUFFER,app->vbo);
	glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);
	glViewport(0,0,ANativeWindow_getWidth(app->window),ANativeWindow_getHeight(app->window));
	return 1;
}

static void term_gl(App* app){
	if(app->display!=EGL_NO_DISPLAY){
		glDeleteBuffers(1,&app->vbo);
		glDeleteProgram(app->prog);
		eglMakeCurrent(app->display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
		if(app->context!=EGL_NO_CONTEXT) eglDestroyContext(app->display,app->context);
		if(app->surface!=EGL_NO_SURFACE) eglDestroySurface(app->display,app->surface);
		eglTerminate(app->display);
	}
	app->display=EGL_NO_DISPLAY; app->context=EGL_NO_CONTEXT; app->surface=EGL_NO_SURFACE;
}

static void* render_thread(void* arg){
	App* app=(App*)arg;
	if(!init_gl(app)) return 0;
	while(app->running){
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(app->prog);
		glBindBuffer(GL_ARRAY_BUFFER,app->vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);
		glDrawArrays(GL_TRIANGLES,0,3);
		eglSwapBuffers(app->display,app->surface);
		usleep(16000);
	}
	term_gl(app);
	return 0;
}

static void onStart(ANativeActivity* a){}
static void onResume(ANativeActivity* a){}
static void* onSave(ANativeActivity* a,size_t* outSize){*outSize=0;return 0;}
static void onPause(ANativeActivity* a){}
static void onStop(ANativeActivity* a){}
static void onDestroy(ANativeActivity* a){App* app=(App*)a->instance;}

static void onWindowFocusChanged(ANativeActivity* a,int focused){}

static void onNativeWindowCreated(ANativeActivity* a,ANativeWindow* w){
	App* app=(App*)a->instance;
	app->window=w;
	app->running=1;
	pthread_create(&app->thread,0,render_thread,app);
}

static void onNativeWindowDestroyed(ANativeActivity* a,ANativeWindow* w){
	App* app=(App*)a->instance;
	app->running=0;
	pthread_join(app->thread,0);
	app->window=0;
}

static void onNativeWindowRedrawNeeded(ANativeActivity* a,ANativeWindow* w){}
static void onInputQueueCreated(ANativeActivity* a,AInputQueue* q){}
static void onInputQueueDestroyed(ANativeActivity* a,AInputQueue* q){}
static void onConfigurationChanged(ANativeActivity* a){}
static void onLowMemory(ANativeActivity* a){}

void ANativeActivity_onCreate(ANativeActivity* a, void* saved, size_t saved_size){
	App* app = (App*)calloc(1,sizeof(App));
	a->instance=app;
	app->activity=a;
	a->callbacks->onStart=onStart;
	a->callbacks->onResume=onResume;
	a->callbacks->onSaveInstanceState=onSave;
	a->callbacks->onPause=onPause;
	a->callbacks->onStop=onStop;
	a->callbacks->onDestroy=onDestroy;
	a->callbacks->onNativeWindowCreated=onNativeWindowCreated;
	a->callbacks->onNativeWindowDestroyed=onNativeWindowDestroyed;
	a->callbacks->onNativeWindowRedrawNeeded=onNativeWindowRedrawNeeded;
	a->callbacks->onWindowFocusChanged=onWindowFocusChanged;
	a->callbacks->onInputQueueCreated=onInputQueueCreated;
	a->callbacks->onInputQueueDestroyed=onInputQueueDestroyed;
	a->callbacks->onConfigurationChanged=onConfigurationChanged;
	a->callbacks->onLowMemory=onLowMemory;
}

