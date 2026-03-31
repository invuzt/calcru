#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

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

// Fungsi mengambil Baterai lewat JNI (Jalur Resmi Android)
int get_native_battery(struct android_app* app) {
    JNIEnv* env;
    (*app->activity->vm)->AttachCurrentThread(app->activity->vm, &env, NULL);

    jclass cls_context = (*env)->FindClass(env, "android/content/Context");
    jfieldID fid_bat = (*env)->GetStaticFieldID(env, cls_context, "BATTERY_SERVICE", "Ljava/lang/String;");
    jobject str_bat = (*env)->GetStaticObjectField(env, cls_context, fid_bat);
    
    jmethodID mid_getSysSrv = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, app->activity->clazz), "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject bat_mgr = (*env)->CallObjectMethod(env, app->activity->clazz, mid_getSysSrv, str_bat);

    jclass cls_batMgr = (*env)->FindClass(env, "android/os/BatteryManager");
    jmethodID mid_getIntProp = (*env)->GetMethodID(env, cls_batMgr, "getIntProperty", "(I)I");
    
    // 4 = BATTERY_PROPERTY_CAPACITY
    int level = (*env)->CallIntMethod(env, bat_mgr, mid_getIntProp, 4);

    (*app->activity->vm)->DetachCurrentThread(app->activity->vm);
    return level;
}

void draw_bar(float y, float val, float r, float g, float b, int win_w, int win_h) {
    glScissor((int)(0.1f * win_w), (int)(y * win_h), (int)(0.8f * win_w), (int)(0.06f * win_h));
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
    if(val > 0.01f) {
        glScissor((int)(0.1f * win_w), (int)(y * win_h), (int)(val * 0.8f * win_w), (int)(0.06f * win_h));
        glClearColor(r, g, b, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
    }
}

static void draw_frame(struct engine* engine) {
    if (engine->display == EGL_NO_DISPLAY) return;

    // Update data asli
    int bat = get_native_battery(engine->app);
    static int cpu_tick = 0; cpu_tick++;
    
    // Kirim ke Rust: CPU(Animasi), RAM(Simulasi), BAT(Asli), TEMP(Simulasi)
    set_sys_data((cpu_tick % 100), 45, bat, 38);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    draw_bar(0.80f, get_cpu_usage(), 0.9f, 0.2f, 0.2f, engine->width, engine->height); // CPU
    draw_bar(0.70f, get_ram_usage(), 0.2f, 0.5f, 1.0f, engine->width, engine->height); // RAM
    draw_bar(0.60f, get_bat_level(), 0.2f, 1.0f, 0.4f, engine->width, engine->height); // BATTERY ASLI
    draw_bar(0.50f, get_temp_level(), 1.0f, 0.7f, 0.1f, engine->width, engine->height); // TEMP

    glDisable(GL_SCISSOR_TEST);
    eglSwapBuffers(engine->display, engine->surface);
}

// ... handle_cmd & android_main tetap sama ...
