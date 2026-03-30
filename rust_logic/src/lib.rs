use std::os::raw::c_float;

const COUNT: usize = 100;
const MAX_TOUCH: usize = 10;

static mut BOXES_X: [f32; COUNT] = [0.5; COUNT];
static mut BOXES_Y: [f32; COUNT] = [0.5; COUNT];
static mut VEL_X: [f32; COUNT] = [0.0; COUNT];
static mut VEL_Y: [f32; COUNT] = [0.0; COUNT];
static mut TOUCH_X: [f32; MAX_TOUCH] = [-1.0; MAX_TOUCH];
static mut TOUCH_Y: [f32; MAX_TOUCH] = [-1.0; MAX_TOUCH];

#[no_mangle]
pub extern "C" fn set_rust_touch(id: i32, x: f32, y: f32) {
    unsafe {
        if id >= 0 && id < MAX_TOUCH as i32 {
            TOUCH_X[id as usize] = x;
            TOUCH_Y[id as usize] = y;
        }
    }
}

#[no_mangle]
pub extern "C" fn update_physics() {
    unsafe {
        for i in 0..COUNT {
            BOXES_X[i] += VEL_X[i];
            BOXES_Y[i] += VEL_Y[i];

            for t in 0..MAX_TOUCH {
                if TOUCH_X[t] < 0.0 { continue; }
                let dx = TOUCH_X[t] - BOXES_X[i];
                let dy = TOUCH_Y[t] - BOXES_Y[i];
                let dist = (dx*dx + dy*dy).sqrt();
                if dist < 0.2 {
                    VEL_X[i] += dx * 0.005;
                    VEL_Y[i] += dy * 0.005;
                }
            }
            VEL_X[i] *= 0.95;
            VEL_Y[i] *= 0.95;
            
            // Batas layar agar tidak hilang
            if BOXES_X[i] < 0.0 || BOXES_X[i] > 1.0 { VEL_X[i] *= -1.0; }
            if BOXES_Y[i] < 0.0 || BOXES_Y[i] > 1.0 { VEL_Y[i] *= -1.0; }
        }
    }
}

#[no_mangle] pub extern "C" fn get_box_x(i: i32) -> f32 { unsafe { BOXES_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_box_y(i: i32) -> f32 { unsafe { BOXES_Y[i as usize] } }
#[no_mangle] pub extern "C" fn get_rust_color_r(t: f32) -> f32 { (t * 0.1).cos().abs() * 0.2 }
