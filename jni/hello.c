#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android/log.h>

#define LOG_TAG "OdfizSystem"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))

extern void set_sys_data(int c, int r, int b, int t);
extern float get_cpu_usage();
extern float get_ram_usage();
extern float get_bat_level();
extern float get_temp_level();

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
};

// Fungsi JNI untuk ambil Baterai
int get_native_battery(struct android_app* app) {
    if (!app || !app->activity || !app->activity->vm) return 50;
    JNIEnv* env;
    (*app->activity->vm)->AttachCurrentThread(app->activity->vm, &env, NULL);

    jclass cls_context = (*env)->FindClass(env, "android/content/Context");
    jfieldID fid_bat = (*env)->GetStaticFieldID(env, cls_context, "BATTERY_SERVICE", "Ljava/lang/String;");
    jobject str_bat = (*env)->GetStaticObjectField(env, cls_context, fid_bat);
    
    jmethodID mid_getSysSrv = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, app->activity->clazz), "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject bat_mgr = (*env)->CallObjectMethod(env, app->activity->clazz, mid_getSysSrv, str_bat);

    jclass cls_batMgr = (*env)->FindClass(env, "android/os/BatteryManager");
    jmethodID mid_getIntProp = (*env)->GetMethodID(env, cls_batMgr, "getIntProperty", "(I)I");
    
    int level = (*env)->CallIntMethod(env, bat_mgr, mid_getIntProp, 4); // 4 = CAPACITY

    (*app->activity->vm)->DetachCurrentThread(app->activity->vm);
    return level;
}

void draw_bar(float y, float val, float r, float g, float b, int win_w, int win_h) {
    if (win_w <= 0 || win_h <= 0) return;
    glScissor((int)(0.1f * win_w), (int)(y * win_h), (int)(0.8f * win_w), (int)(0.06f * win_h));
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
    if(val > 0.01f) {
        glScissor((int)(0.1f * win_w), (int)(y * win_h), (int)(val * 0.8f * win_w), (int)(0.06f * win_h));
        glClearColor(r, g, b, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
    }
}

static void draw_frame(struct engine* engine) {
    if (engine->display == EGL_NO_DISPLAY) return;

    int bat = get_native_battery(engine->app);
    static int cpu_tick = 0; cpu_tick++;
    set_sys_data((cpu_tick % 100), 45, bat, 38);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    draw_bar(0.80f, get_cpu_usage(), 0.9f, 0.2f, 0.2f, engine->width, engine->height);
    draw_bar(0.70f, get_ram_usage(), 0.2f, 0.5f, 1.0f, engine->width, engine->height);
    draw_bar(0.60f, get_bat_level(), 0.2f, 1.0f, 0.4f, engine->width, engine->height);
    draw_bar(0.50f, get_temp_level(), 1.0f, 0.7f, 0.1f, engine->width, engine->height);

    glDisable(GL_SCISSOR_TEST);
    eglSwapBuffers(engine->display, engine->surface);
}

static int init_display(struct engine* engine) {
    engine->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(engine->display, 0, 0);
    EGLConfig config; EGLint num;
    eglChooseConfig(engine->display, (EGLint[]){EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_BLUE_SIZE, 8, EGL_NONE}, &config, 1, &num);
    engine->surface = eglCreateWindowSurface(engine->display, config, engine->app->window, NULL);
    engine->context = eglCreateContext(engine->display, config, NULL, (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
    eglMakeCurrent(engine->display, engine->surface, engine->surface, engine->context);
    eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &engine->width);
    eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &engine->height);
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
    engine.app = state;
    engine.display = EGL_NO_DISPLAY;
    state->userData = &engine;
    state->onAppCmd = handle_cmd;

    while (1) {
        int id, events;
        struct android_poll_source* source;
        while ((id = ALooper_pollOnce(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }
        if (engine.display != EGL_NO_DISPLAY) draw_frame(&engine);
    }
}
