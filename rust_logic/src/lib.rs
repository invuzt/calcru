use std::os::raw::c_float;

const COUNT: usize = 100;
const MAX_TOUCH: usize = 5;

static mut BOXES_X: [f32; COUNT] = [0.5; COUNT];
static mut BOXES_Y: [f32; COUNT] = [0.5; COUNT];
static mut VEL_X: [f32; COUNT] = [0.0; COUNT];
static mut VEL_Y: [f32; COUNT] = [0.0; COUNT];

// Array untuk menyimpan posisi 5 jari
static mut TOUCH_POINTS_X: [f32; MAX_TOUCH] = [-1.0; MAX_TOUCH];
static mut TOUCH_POINTS_Y: [f32; MAX_TOUCH] = [-1.0; MAX_TOUCH];

#[no_mangle]
pub extern "C" fn set_rust_multi_touch(id: i32, x: f32, y: f32) {
    unsafe {
        if id >= 0 && id < MAX_TOUCH as i32 {
            TOUCH_POINTS_X[id as usize] = x;
            TOUCH_POINTS_Y[id as usize] = y;
        }
    }
}

#[no_mangle]
pub extern "C" fn update_physics() {
    unsafe {
        for i in 0..COUNT {
            BOXES_X[i] += VEL_X[i];
            BOXES_Y[i] += VEL_Y[i];

            // Pantulan dinding
            if BOXES_X[i] < 0.0 || BOXES_X[i] > 1.0 { VEL_X[i] *= -0.9; }
            if BOXES_Y[i] < 0.0 || BOXES_Y[i] > 1.0 { VEL_Y[i] *= -0.9; }

            // Cek interaksi dengan SEMUA jari (5 jari)
            for t in 0..MAX_TOUCH {
                let tx = TOUCH_POINTS_X[t];
                let ty = TOUCH_POINTS_Y[t];
                if tx < 0.0 { continue; } // Jari tidak menempel

                let dx = tx - BOXES_X[i];
                let dy = ty - BOXES_Y[i];
                let dist = (dx*dx + dy*dy).sqrt();
                if dist < 0.25 {
                    VEL_X[i] += dx * 0.002;
                    VEL_Y[i] += dy * 0.002;
                }
            }
            
            // Gesekan agar tidak terlalu liar
            VEL_X[i] *= 0.98;
            VEL_Y[i] *= 0.98;
        }
    }
}

#[no_mangle] pub extern "C" fn get_box_x(i: i32) -> f32 { unsafe { BOXES_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_box_y(i: i32) -> f32 { unsafe { BOXES_Y[i as usize] } }
#[no_mangle] pub extern "C" fn get_rust_color_r(t: f32) -> f32 { (t * 0.1).sin().abs() * 0.1 }
