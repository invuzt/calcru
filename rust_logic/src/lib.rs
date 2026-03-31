use std::os::raw::{c_int, c_float};

static mut PLAYER_X: f32 = 0.5;
static mut ENEMY_X: f32 = 0.5;
static mut ENEMY_Y: f32 = 1.2;
static mut SCORE: i32 = 0;
static mut IS_GAMEOVER: bool = false;

// Sistem Partikel Pecah
const MAX_P: usize = 10;
static mut P_X: [f32; MAX_P] = [0.0; MAX_P];
static mut P_Y: [f32; MAX_P] = [0.0; MAX_P];
static mut P_VX: [f32; MAX_P] = [0.0; MAX_P];
static mut P_VY: [f32; MAX_P] = [0.0; MAX_P];
static mut P_LIFE: f32 = 0.0;

#[no_mangle]
pub extern "C" fn update_game(touch_x: f32) -> i32 {
    unsafe {
        if IS_GAMEOVER {
            if P_LIFE > 0.0 {
                P_LIFE -= 0.02;
                for i in 0..MAX_P {
                    P_X[i] += P_VX[i];
                    P_Y[i] += P_VY[i];
                }
            }
            if touch_x > 0.4 && touch_x < 0.6 { // Reset
                IS_GAMEOVER = false; SCORE = 0; ENEMY_Y = 1.2; P_LIFE = 0.0;
            }
            return -2; 
        }

        if touch_x >= 0.0 { PLAYER_X = touch_x; }
        ENEMY_Y -= 0.02;

        if ENEMY_Y < -0.1 { ENEMY_Y = 1.2; ENEMY_X = (SCORE as f32 * 0.7) % 0.8 + 0.1; SCORE += 1; }

        let dx = (PLAYER_X - ENEMY_X).abs();
        let dy = (0.15 - ENEMY_Y).abs();
        
        if dx < 0.12 && dy < 0.12 {
            IS_GAMEOVER = true;
            P_LIFE = 1.0;
            // Inisialisasi pecahan (pecah berkeping-keping)
            for i in 0..MAX_P {
                P_X[i] = ENEMY_X; P_Y[i] = ENEMY_Y;
                P_VX[i] = (i as f32 * 0.6).cos() * 0.03;
                P_VY[i] = (i as f32 * 0.6).sin() * 0.03;
            }
            return -1; // Trigger Bunyi Nabrak!
        }
        SCORE
    }
}

#[no_mangle] pub extern "C" fn get_player_x() -> f32 { unsafe { PLAYER_X } }
#[no_mangle] pub extern "C" fn get_enemy_x() -> f32 { unsafe { ENEMY_X } }
#[no_mangle] pub extern "C" fn get_enemy_y() -> f32 { unsafe { ENEMY_Y } }
#[no_mangle] pub extern "C" fn get_p_x(i: i32) -> f32 { unsafe { P_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_p_y(i: i32) -> f32 { unsafe { P_Y[i as usize] } }
#[no_mangle] pub extern "C" fn get_p_life() -> f32 { unsafe { P_LIFE } }
#[no_mangle] pub extern "C" fn check_game_over() -> bool { unsafe { IS_GAMEOVER } }
