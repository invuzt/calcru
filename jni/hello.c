#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android/log.h>
#include <stdint.h>
#include "sound.h"

#define LOG_TAG "CAKRU_GAME"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern int update_game(float x);
extern float get_player_x();
extern float get_enemy_x();
extern float get_enemy_y();
extern float get_p_x(int i);
extern float get_p_y(int i);
extern float get_p_life();
extern int check_game_over();

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    float last_touch_x;
    int animating;
};

void draw_box(float x, float y, float w_pct, float h_pct, float r, float g, float b, int win_w, int win_h) {
    glScissor((int)(x * win_w), (int)(y * win_h), (int)(w_pct * win_w), (int)(h_pct * win_h));
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void draw_frame(struct engine* engine) {
    if (engine->display == NULL) return;
    int32_t w, h;
    eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &w);
    eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &h);

    int status = update_game(engine->last_touch_x);
    if (status == -1) {
        LOGI("BOOM! Nabrak!");
        play_crash_sound();
    }

    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    if (check_game_over()) glClearColor(0.4f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    if (!check_game_over()) {
        draw_box(get_player_x() - 0.05f, 0.15f, 0.15f, 0.08f, 0.0f, 0.6f, 1.0f, w, h);
        draw_box(get_enemy_x() - 0.05f, get_enemy_y(), 0.12f, 0.07f, 1.0f, 0.8f, 0.0f, w, h);
    } else {
        for(int i=0; i<10; i++) {
            draw_box(get_p_x(i), get_p_y(i), 0.03f, 0.02f, 1.0f, 0.4f, 0.0f, w, h);
        }
    }

    for(int i=0; i < (status > 0 ? status : 0); i++) {
        draw_box(0.05f + (i * 0.04f), 0.92f, 0.02f, 0.02f, 1.0f, 1.0f, 1.0f, w, h);
    }

    glDisable(GL_SCISSOR_TEST);
    eglSwapBuffers(engine->display, engine->surface);
}

static int init_display(struct engine* engine) {
    const EGLint attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE };
    EGLConfig config; EGLint numConfigs, format;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);
    engine->surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    engine->context = eglCreateContext(display, config, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
    eglMakeCurrent(display, engine->surface, engine->surface, engine->context);
    engine->display = display;
    return 0;
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->last_touch_x = AMotionEvent_getX(event, 0) / ANativeWindow_getWidth(app->window);
        return 1;
    }
    return 0;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) { init_display(engine); engine->animating = 1; }
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

    init_sound();

    while (1) {
        int id, events; struct android_poll_source* source;
        while ((id = ALooper_pollOnce(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }
        if (engine.animating) draw_frame(&engine);
    }
}
