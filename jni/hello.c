#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

extern void set_rust_touch(int id, float x, float y);
extern void update_physics();
extern float get_box_x(int i);
extern float get_box_y(int i);
extern float get_rust_color_r(float t);

struct engine {
    struct android_app* app;
    EGLDisplay display; EGLSurface surface; EGLContext context;
    int animating; float tick;
};

// Fungsi bantuan untuk memicu feedback getaran/suara sistem (Simple Haptic)
static void trigger_feedback(struct engine* eng) {
    // Kita gunakan haptic feedback sebagai pengganti audio file eksternal agar tidak ribet urusan path file
    // Di level Native, ini cara tercepat untuk "bunyi" klik/getar
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* eng = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event);
        int action_code = action & AMOTION_EVENT_ACTION_MASK;
        int pointer_idx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        
        int32_t w = ANativeWindow_getWidth(app->window);
        int32_t h = ANativeWindow_getHeight(app->window);

        if (action_code == AMOTION_EVENT_ACTION_DOWN || action_code == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            trigger_feedback(eng); // Bunyi/Getar setiap jari baru menempel
        }

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

    glClearColor(get_rust_color_r(eng->tick), 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    for(int i = 0; i < 100; i++) {
        int bx = (int)(get_box_x(i) * w) - 15;
        int by = (int)((1.0f - get_box_y(i)) * h) - 15;
        glScissor(bx, by, 30, 30);
        glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
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
        case APP_CMD_TERM_WINDOW: eng->animating = 0; break;
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
