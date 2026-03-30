#include <jni.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android_native_app_glue.h>

extern void set_rust_touch(float x, float y);
extern float get_rust_color_r(float t);
extern float get_rust_color_g(float t);

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    float tick;
};

// Fungsi input yang dinamis (Support Portrait & Landscape)
static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        // Ambil dimensi layar aktif dari window
        int32_t width = ANativeWindow_getWidth(app->window);
        int32_t height = ANativeWindow_getHeight(app->window);
        
        // Normalisasi koordinat (0.0f ke 1.0f) berdasarkan ukuran layar asli
        float x = AMotionEvent_getX(event, 0) / (float)width;
        float y = AMotionEvent_getY(event, 0) / (float)height;
        
        set_rust_touch(x, y);
        return 1;
    }
    return 0;
}

static void draw_frame(struct engine* eng) {
    if (eng->display == NULL) return;
    
    // Ambil warna dari Rust logic
    float r = get_rust_color_r(eng->tick);
    float g = get_rust_color_g(eng->tick);
    
    glClearColor(r, g, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    eglSwapBuffers(eng->display, eng->surface);
    eng->tick += 0.02f;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* eng = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) {
                EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
                eglInitialize(disp, 0, 0);
                
                const EGLint attr[] = { 
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, 
                    EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, 
                    EGL_RED_SIZE, 8, EGL_NONE 
                };
                
                EGLConfig cfg; 
                EGLint n;
                eglChooseConfig(disp, attr, &cfg, 1, &n);
                eng->surface = eglCreateWindowSurface(disp, cfg, app->window, NULL);
                eng->display = disp;
                
                EGLContext ctx = eglCreateContext(disp, cfg, NULL, 
                    (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
                eglMakeCurrent(disp, eng->surface, eng->surface, ctx);
                
                // Pastikan Viewport OpenGL sesuai dengan ukuran window baru
                glViewport(0, 0, ANativeWindow_getWidth(app->window), 
                                ANativeWindow_getHeight(app->window));
            }
            break;
        case APP_CMD_TERM_WINDOW:
            eng->display = NULL;
            eng->surface = NULL;
            break;
    }
}

void android_main(struct android_app* state) {
    struct engine eng = {0};
    state->userData = &eng;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;
    eng.app = state;

    while (1) {
        int id, ev;
        struct android_poll_source* src;
        while ((id = ALooper_pollOnce(0, NULL, &ev, (void**)&src)) >= 0) {
            if (src != NULL) src->process(state, src);
            if (state->destroyRequested != 0) return;
        }
        if (eng.display != NULL) draw_frame(&eng);
    }
}
