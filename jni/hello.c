#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android/log.h>

#define LOG_TAG "CakruSystem"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))

extern float get_cpu_usage();
extern float get_ram_usage();
extern float get_bat_level();
extern float get_temp_level();

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
};

void draw_box(float x, float y, float w_pct, float h_pct, float r, float g, float b, int win_w, int win_h) {
    // Memastikan koordinat dalam pixel
    int px = (int)(x * win_w);
    int py = (int)(y * win_h);
    int pw = (int)(w_pct * win_w);
    int ph = (int)(h_pct * win_h);
    
    glScissor(px, py, pw, ph);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void draw_frame(struct engine* engine) {
    if (engine->display == NULL) return;

    int32_t w, h;
    eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &w);
    eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &h);

    // Background Utama (Gelap)
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_SCISSOR_TEST);

    float bar_w = 0.7f;
    float start_x = 0.2f;

    // 1. BAR CPU (MERAH)
    float cpu = get_cpu_usage();
    draw_box(0.05f, 0.85f, 0.1f, 0.05f, 0.8f, 0.2f, 0.2f, w, h); // Icon Square
    draw_box(start_x, 0.85f, bar_w, 0.05f, 0.2f, 0.2f, 0.2f, w, h); // BG Bar
    draw_box(start_x, 0.85f, bar_w * cpu, 0.05f, 0.9f, 0.1f, 0.1f, w, h); // Fill

    // 2. BAR RAM (BIRU)
    float ram = get_ram_usage();
    draw_box(0.05f, 0.75f, 0.1f, 0.05f, 0.2f, 0.4f, 0.8f, w, h);
    draw_box(start_x, 0.75f, bar_w, 0.05f, 0.2f, 0.2f, 0.2f, w, h);
    draw_box(start_x, 0.75f, bar_w * ram, 0.05f, 0.1f, 0.5f, 1.0f, w, h);

    // 3. BAR BATERAI (HIJAU)
    float bat = get_bat_level();
    draw_box(0.05f, 0.65f, 0.1f, 0.05f, 0.2f, 0.8f, 0.3f, w, h);
    draw_box(start_x, 0.65f, bar_w, 0.05f, 0.2f, 0.2f, 0.2f, w, h);
    draw_box(start_x, 0.65f, bar_w * bat, 0.05f, 0.2f, 1.0f, 0.4f, w, h);

    // 4. BAR SUHU (ORANYE)
    float temp = get_temp_level();
    draw_box(0.05f, 0.55f, 0.1f, 0.05f, 1.0f, 0.6f, 0.0f, w, h);
    draw_box(start_x, 0.55f, bar_w, 0.05f, 0.2f, 0.2f, 0.2f, w, h);
    draw_box(start_x, 0.55f, bar_w * temp, 0.05f, 1.0f, 0.7f, 0.1f, w, h);

    eglSwapBuffers(engine->display, engine->surface);
}

static int init_display(struct engine* engine) {
    const EGLint attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, 
                               EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, 
                               EGL_RED_SIZE, 8, EGL_NONE };
    EGLConfig config; EGLint numConfigs, format;
    engine->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(engine->display, 0, 0);
    eglChooseConfig(engine->display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(engine->display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);
    engine->surface = eglCreateWindowSurface(engine->display, config, engine->app->window, NULL);
    engine->context = eglCreateContext(engine->display, config, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
    eglMakeCurrent(engine->display, engine->surface, engine->surface, engine->context);
    return 0;
}

static void handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) {
                init_display(engine);
                draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroyContext(engine->display, engine->context);
            eglDestroySurface(engine->display, engine->surface);
            eglTerminate(engine->display);
            engine->display = NULL;
            break;
    }
}

void android_main(struct android_app* state) {
    struct engine engine = {0};
    state->userData = &engine;
    state->onAppCmd = handle_cmd;
    
    while (1) {
        int id, events;
        struct android_poll_source* source;
        while ((id = ALooper_pollOnce(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }
        if (engine.display != NULL) draw_frame(&engine);
    }
}
