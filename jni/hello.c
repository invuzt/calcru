#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

extern float get_cpu_usage();
extern float get_ram_usage();
extern float get_bat_level();
extern float get_temp_level();

void draw_box(float x, float y, float w, float h, float r, float g, float b, int win_w, int win_h) {
    glScissor((int)(x * win_w), (int)(y * win_h), (int)(w * win_w), (int)(h * win_h));
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// Fungsi membuat icon pixel art sederhana
void draw_icon(int type, float x, float y, int w, int h) {
    if(type == 0) { // Icon Baterai (Hijau)
        draw_box(x, y, 0.05f, 0.03f, 0.2f, 0.8f, 0.2f, w, h);
        draw_box(x+0.05f, y+0.01f, 0.01f, 0.01f, 0.2f, 0.8f, 0.2f, w, h);
    } else if(type == 1) { // Icon CPU (Merah - bentuk Chip)
        draw_box(x, y, 0.04f, 0.04f, 0.8f, 0.2f, 0.2f, w, h);
        draw_box(x+0.01f, y+0.01f, 0.02f, 0.02f, 0.1f, 0.1f, 0.1f, w, h);
    }
}

static void draw_frame(struct android_app* app) {
    int32_t w = ANativeWindow_getWidth(app->window);
    int32_t h = ANativeWindow_getHeight(app->window);

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    float padding = 0.1f;
    float bar_w = 0.7f;

    // --- 1. CPU (Merah) ---
    draw_icon(1, 0.05f, 0.85f, w, h);
    float cpu = get_cpu_usage();
    draw_box(0.15f, 0.85f, bar_w, 0.03f, 0.2f, 0.2f, 0.2f, w, h); // Background bar
    draw_box(0.15f, 0.85f, bar_w * cpu, 0.03f, 0.8f, 0.2f, 0.2f, w, h); // Isi bar

    // --- 2. RAM (Biru) ---
    draw_box(0.05f, 0.75f, 0.04f, 0.04f, 0.2f, 0.4f, 0.8f, w, h); // Icon RAM
    float ram = get_ram_usage();
    draw_box(0.15f, 0.75f, bar_w, 0.03f, 0.2f, 0.2f, 0.2f, w, h);
    draw_box(0.15f, 0.75f, bar_w * ram, 0.03f, 0.2f, 0.4f, 0.8f, w, h);

    // --- 3. BATERAI (Hijau) ---
    draw_icon(0, 0.05f, 0.65f, w, h);
    float bat = get_bat_level();
    draw_box(0.15f, 0.65f, bar_w, 0.03f, 0.2f, 0.2f, 0.2f, w, h);
    draw_box(0.15f, 0.65f, bar_w * bat, 0.03f, 0.2f, 0.8f, 0.2f, w, h);

    // --- 4. SUHU (Oranye) ---
    draw_box(0.06f, 0.55f, 0.02f, 0.04f, 1.0f, 0.5f, 0.0f, w, h); // Icon Termometer
    float temp = get_temp_level();
    draw_box(0.15f, 0.55f, bar_w, 0.03f, 0.2f, 0.2f, 0.2f, w, h);
    draw_box(0.15f, 0.55f, bar_w * temp, 0.03f, 1.0f, 0.5f, 0.0f, w, h);

    glDisable(GL_SCISSOR_TEST);
}

void android_main(struct android_app* state) {
    // ... (Logika init EGL sama seperti sebelumnya) ...
    // Untuk singkatnya, pastikan loop memanggil draw_frame(state)
}
