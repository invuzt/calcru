#include <jni.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android_native_app_glue.h>

extern void set_rust_touch(float x, float y);
extern float get_box_x();
extern float get_box_y();
extern float get_rust_color_r(float t);

struct engine {
    struct android_app* app;
    EGLDisplay display; EGLSurface surface;
    float tick;
};

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t w = ANativeWindow_getWidth(app->window);
        int32_t h = ANativeWindow_getHeight(app->window);
        set_rust_touch(AMotionEvent_getX(event, 0) / (float)w, 
                       AMotionEvent_getY(event, 0) / (float)h);
        return 1;
    }
    return 0;
}

static void draw_frame(struct engine* eng) {
    if (eng->display == NULL) return;
    int32_t w = ANativeWindow_getWidth(eng->app->window);
    int32_t h = ANativeWindow_getHeight(eng->app->window);

    // 1. Gambar Background (Warna dari Rust)
    glClearColor(get_rust_color_r(eng->tick), 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 2. Gambar Kotak Interaktif (Posisi dari Rust)
    glEnable(GL_SCISSOR_TEST);
    int box_size = 200;
    int bx = (int)(get_box_x() * w) - (box_size / 2);
    int by = (int)((1.0f - get_box_y()) * h) - (box_size / 2); // OpenGL koordinat Y terbalik
    
    glScissor(bx, by, box_size, box_size);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Warna kotak putih
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    eglSwapBuffers(eng->display, eng->surface);
    eng->tick += 0.01f;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* eng = (struct engine*)app->userData;
    if (cmd == APP_CMD_INIT_WINDOW) {
        EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(disp, 0, 0);
        EGLConfig cfg; EGLint n;
        eglChooseConfig(disp, (EGLint[]){EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE}, &cfg, 1, &n);
        eng->surface = eglCreateWindowSurface(disp, cfg, app->window, NULL);
        eng->display = disp;
        EGLContext ctx = eglCreateContext(disp, cfg, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
        eglMakeCurrent(disp, eng->surface, eng->surface, ctx);
        glViewport(0, 0, ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));
    }
}

void android_main(struct android_app* state) {
    struct engine eng = {0};
    state->userData = &eng;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;
    while (1) {
        int id, ev; struct android_poll_source* src;
        while ((id = ALooper_pollOnce(0, NULL, &ev, (void**)&src)) >= 0) {
            if (src != NULL) src->process(state, src);
            if (state->destroyRequested != 0) return;
        }
        if (eng.display != NULL) draw_frame(&eng);
    }
}
