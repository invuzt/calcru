#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// Fungsi dari Rust
extern void set_rust_touch(float x, float y);
extern void update_physics();
extern float get_box_x(int i);
extern float get_box_y(int i);
extern float get_rust_color_r(float t);

struct engine {
    struct android_app* app;
    EGLDisplay display; EGLSurface surface; EGLContext context;
    int animating; float tick;
};

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event);
        int32_t w = ANativeWindow_getWidth(app->window);
        int32_t h = ANativeWindow_getHeight(app->window);
        
        // Jari menempel atau bergerak
        if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_MOVE) {
            set_rust_touch(AMotionEvent_getX(event, 0)/w, AMotionEvent_getY(event, 0)/h);
        } else if (action == AMOTION_EVENT_ACTION_UP) {
            set_rust_touch(-1.0f, -1.0f); // Jari diangkat
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

    // Latar belakang biru langit gelap
    glClearColor(0.0f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    for(int i = 0; i < 150; i++) { // Gunakan jumlah partikel yang baru (150)
        int bx = (int)(get_box_x(i) * w) - 15;
        int by = (int)((1.0f - get_box_y(i)) * h) - 2;
        
        // Bentuk "Garis Angin" (Panjang 30px, Tinggi 4px)
        glScissor(bx, by, 30, 4);
        
        // Warna putih transparan/cyan lembut
        glClearColor(0.6f, 1.0f, 1.0f, 0.6f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);
    eglSwapBuffers(eng->display, eng->surface);
    eng->tick += 0.01f;
}

// ... sisanya (handle_cmd, android_main) sama seperti sebelumnya ...
// Pastikan fungsi handle_cmd dan android_main Anda tidak terhapus ya!
