#include <jni.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <stdint.h>

extern int update_game(float x);
extern float get_player_x();
extern float get_enemy_x();
extern float get_enemy_y();
extern float get_p_x(int i);
extern float get_p_y(int i);
extern float get_p_life();
extern int check_game_over();

struct engine { struct android_app* app; EGLDisplay d; EGLSurface s; EGLContext c; float tx; int anim; };

// Gambar kotak kecil (untuk pecahan atau angka)
void draw_box(float x, float y, float w_size, float h_size, float r, float g, float b, int win_w, int win_h) {
    glScissor((int)(x * win_w), (int)(y * win_h), (int)w_size, (int)h_size);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void draw_frame(struct engine* engine) {
    if (engine->d == NULL) return;
    int32_t w, h;
    eglQuerySurface(engine->d, engine->s, EGL_WIDTH, &w);
    eglQuerySurface(engine->d, engine->s, EGL_HEIGHT, &h);

    int status = update_game(engine->tx);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    if (check_game_over()) glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    if (!check_game_over()) {
        draw_box(get_player_x() - 0.05, 0.15, 100, 100, 0.0, 0.6, 1.0, w, h); // Player
        draw_box(get_enemy_x() - 0.04, get_enemy_y(), 80, 80, 1.0, 0.8, 0.0, w, h); // Enemy
    } else {
        // Pecahan berkeping-keping
        float life = get_p_life();
        for(int i=0; i<10; i++) {
            draw_box(get_p_x(i), get_p_y(i), 30, 30, 1.0, 0.5, 0.0, w, h);
        }
    }

    // Skor Sederhana (Kotak di pojok)
    for(int i=0; i <= (status > 0 ? status : 0) % 10; i++) {
        draw_box(0.05 + (i * 0.03), 0.9, 30, 30, 1.0, 1.0, 1.0, w, h);
    }

    glDisable(GL_SCISSOR_TEST);
    eglSwapBuffers(engine->d, engine->s);
}

// ... (Sisanya: init_display, handle_input, android_main seperti sebelumnya) ...
// Copy paste android_main dan handler dari file jni/hello.c sebelumnya ya!
