#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android/log.h>
#include <sys/stat.h>
#include <unistd.h>

#define LOG_TAG "OdfizSystem"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))

extern float get_cpu_usage();
extern float get_ram_usage();
extern float get_bat_level();
extern float get_temp_level();
extern void load_stok();

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
};

void draw_box(float x, float y, float w_pct, float h_pct, float r, float g, float b, int win_w, int win_h) {
    if (win_w <= 0 || win_h <= 0) return;
    glScissor((int)(x * win_w), (int)(y * win_h), (int)(w_pct * win_w), (int)(h_pct * win_h));
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void draw_frame(struct engine* engine) {
    if (engine->display == EGL_NO_DISPLAY || engine->surface == EGL_NO_SURFACE) return;

    int32_t w, h;
    eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &w);
    eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &h);

    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_SCISSOR_TEST);
    float bar_w = 0.7f;
    float start_x = 0.2f;

    // Drawing Bars
    draw_box(start_x, 0.85f, bar_w * get_cpu_usage(), 0.05f, 0.9f, 0.1f, 0.1f, w, h); // CPU
    draw_box(start_x, 0.75f, bar_w * get_ram_usage(), 0.05f, 0.1f, 0.5f, 1.0f, w, h); // RAM
    draw_box(start_x, 0.65f, bar_w * get_bat_level(), 0.05f, 0.2f, 1.0f, 0.4f, w, h); // BAT
    draw_box(start_x, 0.55f, bar_w * get_temp_level(), 0.05f, 1.0f, 0.7f, 0.1f, w, h); // TEMP

    eglSwapBuffers(engine->display, engine->surface);
}

static int init_display(struct engine* engine) {
    engine->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(engine->display, 0, 0);
    
    const EGLint attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_BLUE_SIZE, 8, EGL_NONE };
    EGLConfig config; EGLint numConfigs;
    eglChooseConfig(engine->display, attribs, &config, 1, &numConfigs);
    
    engine->surface = eglCreateWindowSurface(engine->display, config, engine->app->window, NULL);
    engine->context = eglCreateContext(engine->display, config, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
    
    if (eglMakeCurrent(engine->display, engine->surface, engine->surface, engine->context) == EGL_FALSE) return -1;
    return 0;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) init_display(engine);
            break;
        case APP_CMD_TERM_WINDOW:
            engine->display = EGL_NO_DISPLAY;
            break;
    }
}

void android_main(struct android_app* state) {
    struct engine engine = {0};
    state->userData = &engine;
    state->onAppCmd = handle_cmd;

    // Pastikan folder files ada sebelum Rust dipanggil
    mkdir("/data/user/0/com.cakru.dodge/files", 0700);
    load_stok(); 

    while (1) {
        int id, events;
        struct android_poll_source* source;
        while ((id = ALooper_pollOnce(engine.display != EGL_NO_DISPLAY ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }
        if (engine.display != EGL_NO_DISPLAY) draw_frame(&engine);
    }
}
