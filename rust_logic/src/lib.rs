use std::os::raw::c_float;

const COUNT: usize = 1000; 

static mut BOXES_X: [f32; COUNT] = [0.0; COUNT];
static mut BOXES_Y: [f32; COUNT] = [0.0; COUNT];
static mut VEL_X: [f32; COUNT] = [0.0; COUNT];
static mut VEL_Y: [f32; COUNT] = [0.0; COUNT];
static mut TOUCH_X: f32 = -1.0;
static mut TOUCH_Y: f32 = -1.0;
static mut INITIALIZED: bool = false;

fn init() {
    unsafe {
        for i in 0..COUNT {
            // Posisi X diacak sedikit agar tidak terlalu kaku
            BOXES_X[i] = (i as f32 * 0.73) % 1.0;
            // Posisi Y didistribusikan SECARA RATA dari atas sampai bawah (Rapat!)
            BOXES_Y[i] = i as f32 / COUNT as f32;
            
            // Kecepatan angin dasar (ke kanan)
            VEL_X[i] = 0.006 + (i as f32 * 0.00001);
            VEL_Y[i] = 0.0;
        }
        INITIALIZED = true;
    }
}

#[no_mangle]
pub extern "C" fn set_rust_touch(x: c_float, y: c_float) {
    unsafe { TOUCH_X = x; TOUCH_Y = y; }
}

#[no_mangle]
pub extern "C" fn update_physics() {
    unsafe {
        if !INITIALIZED { init(); }
        for i in 0..COUNT {
            if TOUCH_X > 0.0 {
                let dx = BOXES_X[i] - TOUCH_X;
                let dy = BOXES_Y[i] - TOUCH_Y;
                let dist_sq = dx*dx + dy*dy;
                let dist = dist_sq.sqrt();
                
                if dist < 0.22 {
                    let force = 0.0006 / (dist + 0.015);
                    // Tolakan kuat agar lubang terlihat jelas
                    VEL_X[i] += (dx / dist) * force;
                    VEL_Y[i] += (dy / dist) * force;
                    // Vortex (Putaran) halus
                    VEL_X[i] -= (dy / dist) * force * 0.3;
                    VEL_Y[i] += (dx / dist) * force * 0.3;
                }
            }
            
            BOXES_X[i] += VEL_X[i];
            BOXES_Y[i] += VEL_Y[i];
            
            // Kembalikan VEL_X ke kecepatan dasar
            VEL_X[i] = VEL_X[i] * 0.96 + 0.0055 * 0.04;
            VEL_Y[i] *= 0.94; // Redaman Y agar kembali lurus

            // Reset Partikel yang keluar layar (Looping)
            if BOXES_X[i] > 1.05 { 
                BOXES_X[i] = -0.05; 
                // Y tetap sama agar tetap rapat
            }
            // Batas Y atas bawah yang ketat
            if BOXES_Y[i] < 0.0 { BOXES_Y[i] = 0.0; VEL_Y[i] *= -0.5; }
            if BOXES_Y[i] > 1.0 { BOXES_Y[i] = 1.0; VEL_Y[i] *= -0.5; }
        }
    }
}

#[no_mangle] pub extern "C" fn get_box_x(i: i32) -> f32 { unsafe { BOXES_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_box_y(i: i32) -> f32 { unsafe { BOXES_Y[i as usize] } }
#[no_mangle] pub extern "C" fn get_rust_color_r(t: f32) -> f32 { (t * 0.05).sin().abs() * 0.05 }
