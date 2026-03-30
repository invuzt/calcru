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
            BOXES_X[i] = (i as f32 * 1.7) % 1.0;
            BOXES_Y[i] = (i as f32 * 3.3) % 1.0;
            VEL_X[i] = 0.008 + (i as f32 * 0.00001);
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
                
                if dist < 0.2 {
                    let force = 0.0008 / (dist + 0.02);
                    VEL_X[i] += (dx / dist) * force;
                    VEL_Y[i] += (dy / dist) * force;
                    // Efek Vortex (Putaran)
                    VEL_X[i] -= (dy / dist) * force * 0.4;
                    VEL_Y[i] += (dx / dist) * force * 0.4;
                }
            }
            
            BOXES_X[i] += VEL_X[i];
            BOXES_Y[i] += VEL_Y[i];
            
            VEL_X[i] = VEL_X[i] * 0.94 + 0.007 * 0.06;
            VEL_Y[i] *= 0.94;

            if BOXES_X[i] > 1.05 { 
                BOXES_X[i] = -0.05; 
                BOXES_Y[i] = (i as f32 * 17.7) % 1.0;
            }
        }
    }
}

#[no_mangle] pub extern "C" fn get_box_x(i: i32) -> f32 { unsafe { BOXES_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_box_y(i: i32) -> f32 { unsafe { BOXES_Y[i as usize] } }
#[no_mangle] pub extern "C" fn get_rust_color_r(t: f32) -> f32 { (t * 0.05).sin().abs() * 0.05 }
