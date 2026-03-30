#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <math.h>

extern void set_rust_touch(int id, float x, float y);
extern void update_physics();
extern float get_box_x(int i);
extern float get_box_y(int i);
extern float get_touch_x(int i);
extern float get_touch_y(int i);
extern float get_rust_color_r(float t);

struct engine {
    struct android_app* app;
    EGLDisplay display; EGLSurface surface; EGLContext context;
    int animating; float tick;
};

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* eng = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event);
        int action_code = action & AMOTION_EVENT_ACTION_MASK;
        int32_t w = ANativeWindow_getWidth(app->window);
        int32_t h = ANativeWindow_getHeight(app->window);

        if (action_code == AMOTION_EVENT_ACTION_UP || action_code == AMOTION_EVENT_ACTION_CANCEL) {
            for(int i=0; i<10; i++) set_rust_touch(i, -1.0f, -1.0f);
        } else {
            int count = AMotionEvent_getPointerCount(event);
            for (int i = 0; i < count && i < 10; i++) {
                set_rust_touch(i, AMotionEvent_getX(event, i)/w, AMotionEvent_getY(event, i)/h);
            }
        }
        return 1;
    }
    return 0;
}

static void draw_frame(struct engine* eng) {
    if (eng->display == EGL_NO_DISPLAY) return;
    update_physics();
    int32_t w, h;
    eglQuerySurface(eng->display, eng->surface, EGL_WIDTH, &w);
    eglQuerySurface(eng->display, eng->surface, EGL_HEIGHT, &h);

    glClearColor(get_rust_color_r(eng->tick), 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    
    // 1. Gambar Garis Antar Jari (Jaring)
    for(int i=0; i<10; i++) {
        for(int j=i+1; j<10; j++) {
            float x1 = get_touch_x(i); float y1 = get_touch_y(i);
            float x2 = get_touch_x(j); float y2 = get_touch_y(j);
            if (x1 < 0 || x2 < 0) continue;
            
            // Gambar kotak kecil di tengah antara dua jari sebagai penghubung
            int mx = (int)(((x1 + x2) / 2.0f) * w) - 2;
            int my = (int)((1.0f - ((y1 + y2) / 2.0f)) * h) - 2;
            glScissor(mx, my, 4, 4);
            glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    // 2. Gambar 100 Partikel
    for(int i = 0; i < 100; i++) {
        int bx = (int)(get_box_x(i) * w) - 10;
        int by = (int)((1.0f - get_box_y(i)) * h) - 10;
        glScissor(bx, by, 20, 20);
        glClearColor(0.2f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    glDisable(GL_SCISSOR_TEST);
    eglSwapBuffers(eng->display, eng->surface);
    eng->tick += 0.01f;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* eng = (struct engine*)app->userData;
    if (cmd == APP_CMD_INIT_WINDOW) {
        eng->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(eng->display, 0, 0);
        EGLConfig cfg; EGLint n;
        eglChooseConfig(eng->display, (EGLint[]){EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE}, &cfg, 1, &n);
        eng->surface = eglCreateWindowSurface(eng->display, cfg, app->window, NULL);
        eng->context = eglCreateContext(eng->display, cfg, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
        eglMakeCurrent(eng->display, eng->surface, eng->surface, eng->context);
        eng->animating = 1;
    } else if (cmd == APP_CMD_TERM_WINDOW) {
        eng->animating = 0;
    }
}

void android_main(struct android_app* state) {
    struct engine eng = {0};
    state->userData = &eng;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;
    while (1) {
        int id, ev; struct android_poll_source* src;
        while ((id = ALooper_pollOnce(eng.animating ? 0 : -1, NULL, &ev, (void**)&src)) >= 0) {
            if (src != NULL) src->process(state, src);
            if (state->destroyRequested != 0) return;
        }
        if (eng.animating) draw_frame(&eng);
    }
}
