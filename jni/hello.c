#include <jni.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android_native_app_glue.h>

// Fungsi dari Rust
extern void set_rust_multi_touch(int id, float x, float y);
extern void update_physics();
extern float get_box_x(int i);
extern float get_box_y(int i);
extern float get_rust_color_r(float t);

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int animating;
    float tick;
};

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* eng = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event);
        int pointer_count = AMotionEvent_getPointerCount(event);
        int32_t w = ANativeWindow_getWidth(app->window);
        int32_t h = ANativeWindow_getHeight(app->window);

        // Jika semua jari diangkat, reset semua posisi touch di Rust
        if ((action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_UP) {
            for(int i = 0; i < 5; i++) set_rust_multi_touch(i, -1.0f, -1.0f);
        } else {
            // Update posisi untuk maksimal 5 jari
            for (int i = 0; i < pointer_count && i < 5; i++) {
                float x = AMotionEvent_getX(event, i) / (float)w;
                float y = AMotionEvent_getY(event, i) / (float)h;
                set_rust_multi_touch(i, x, y);
            }
        }
        return 1;
    }
    return 0;
}

static void draw_frame(struct engine* eng) {
    if (eng->display == EGL_NO_DISPLAY || eng->surface == EGL_NO_SURFACE) return;

    update_physics();

    int32_t w, h;
    eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &w);
    eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &h);

    glClearColor(get_rust_color_r(eng->tick), 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    for(int i = 0; i < 100; i++) {
        int bx = (int)(get_box_x(i) * w) - 12;
        int by = (int)((1.0f - get_box_y(i)) * h) - 12;
        glScissor(bx, by, 24, 24);
        glClearColor(0.2f, 1.0f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);

    eglSwapBuffers(eng->display, eng->surface);
    eng->tick += 0.01f;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* eng = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) {
                eng->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
                eglInitialize(eng->display, 0, 0);
                EGLConfig cfg; EGLint n;
                eglChooseConfig(eng->display, (EGLint[]){EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE}, &cfg, 1, &n);
                eng->surface = eglCreateWindowSurface(eng->display, cfg, app->window, NULL);
                eng->context = eglCreateContext(eng->display, cfg, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
                eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
                eng->animating = 1;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            eng->animating = 0;
            eng->display = EGL_NO_DISPLAY;
            eng->surface = EGL_NO_SURFACE;
            break;
        case APP_CMD_GAINED_FOCUS: eng->animating = 1; break;
        case APP_CMD_LOST_FOCUS: eng->animating = 0; break;
    }
}

// INI BAGIAN YANG TADI HILANG:
void android_main(struct android_app* state) {
    struct engine eng = {0};
    eng.display = EGL_NO_DISPLAY;
    eng.surface = EGL_NO_SURFACE;
    
    state->userData = &eng;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;
    eng.app = state;

    while (1) {
        int id, ev;
        struct android_poll_source* src;
        while ((id = ALooper_pollOnce(eng.animating ? 0 : -1, NULL, &ev, (void**)&src)) >= 0) {
            if (src != NULL) src->process(state, src);
            if (state->destroyRequested != 0) return;
        }
        if (eng.animating && eng.display != EGL_NO_DISPLAY) {
            draw_frame(&eng);
        }
    }
}
