#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android/log.h>

#define LOG_TAG "CAKRU_GAME"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern int update_game(float touch_x);
extern float get_player_x();
extern float get_enemy_x();
extern float get_enemy_y();

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    float last_touch_x;
    int animating;
};

static int init_display(struct engine* engine) {
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
        EGL_NONE
    };
    EGLint format;
    EGLint numConfigs;
    EGLConfig config;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    
    // PERBAIKAN DI SINI: Nama fungsi yang benar adalah eglGetConfigAttrib
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    engine->surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    engine->context = eglCreateContext(display, config, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});

    if (eglMakeCurrent(display, engine->surface, engine->surface, engine->context) == EGL_FALSE) return -1;

    engine->display = display;
    return 0;
}

static void draw_frame(struct engine* engine) {
    if (engine->display == NULL || engine->surface == NULL) return;

    int32_t w, h;
    eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &w);
    eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &h);

    update_game(engine->last_touch_x);

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    // Player (Biru)
    glScissor((int)(get_player_x() * w) - 60, (int)(0.15 * h), 120, 120);
    glClearColor(0.0f, 0.6f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Musuh (Merah)
    glScissor((int)(get_enemy_x() * w) - 50, (int)(get_enemy_y() * h), 100, 100);
    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    eglSwapBuffers(engine->display, engine->surface);
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t w = ANativeWindow_getWidth(app->window);
        // Pastikan lebar tidak nol untuk menghindari pembagian dengan nol
        if (w > 0) {
            engine->last_touch_x = AMotionEvent_getX(event, 0) / (float)w;
        }
        return 1;
    }
    return 0;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) {
                init_display(engine);
                engine->animating = 1;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine->animating = 0;
            break;
    }
}

void android_main(struct android_app* state) {
    struct engine engine = {0};
    engine.app = state;
    engine.last_touch_x = 0.5f;

    state->userData = &engine;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;

    while (1) {
        int id, events;
        struct android_poll_source* source;
        while ((id = ALooper_pollOnce(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }
        if (engine.animating) draw_frame(&engine);
    }
}
