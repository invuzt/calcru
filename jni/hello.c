#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

extern void set_rust_multi_touch(int id, float x, float y);
extern void update_physics();
extern float get_box_x(int i);
extern float get_box_y(int i);

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event);
        int count = AMotionEvent_getPointerCount(event);
        int32_t w = ANativeWindow_getWidth(app->window);
        int32_t h = ANativeWindow_getHeight(app->window);

        // Reset semua jari ke "mati" dulu jika semua dilepas
        if ((action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_UP) {
            for(int i=0; i<5; i++) set_rust_multi_touch(i, -1.0f, -1.0f);
        }

        // Update posisi untuk setiap jari yang sedang menempel
        for (int i = 0; i < count && i < 5; i++) {
            float x = AMotionEvent_getX(event, i) / (float)w;
            float y = AMotionEvent_getY(event, i) / (float)h;
            set_rust_multi_touch(i, x, y);
        }
        return 1;
    }
    return 0;
}

// ... (Gunakan fungsi draw_frame, handle_cmd, dan android_main yang sebelumnya) ...
