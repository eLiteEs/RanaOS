// rust/src/lib.rs
#![no_std]
#![no_main]
#![feature(lang_items)]
#[warn(improper_ctypes)]
#[warn(internal_features)]

mod graphics;
pub use graphics::*;

mod datetime;
pub use datetime::*;

mod debug;
pub use debug::*;

mod ps2;
pub use ps2::*;

// External C functions from your kernel
extern "C" {
    // Wait functions
    fn wait_ms(ms: u32);

    // Run programs
    fn run_elf(name: *const u8);
}

fn _char_to_utf8_str(c: char, buf: &mut [u8; 4]) -> &str {
    let len = c.encode_utf8(buf).len();
    unsafe { core::str::from_utf8_unchecked(&buf[..len]) }
}

#[no_mangle]
pub extern "C" fn start_so() {
    Graphics::clear_screen();
    
    // Draw background
    Graphics::draw_rect(0, 0, Graphics::width(), Graphics::height(), 0x42f590);

    // Initialize mouse
    let width = Graphics::width();
    let height = Graphics::height();
    
    let mut mouse = PS2::new(width, height);
    unsafe { mouse.init() };

    // Draw toolbar
    Graphics::draw_rect(0, 0, Graphics::width(), 20, 0x000000);
    Graphics::draw_string(2, 2, "RanaOS beta 3 | Press \'r\' for opening launch dialog", 0xffffff, 0x000000, false, false);

    // Display datetime in toolbar (right-aligned)
    let datetime = DateTime;
    let datetime_str = datetime.format();
    let time_str = unsafe { core::str::from_utf8_unchecked(&datetime_str) };
    
    let time_width = time_str.len() as u32 * 8;

    Graphics::draw_string(Graphics::width() / 2 - time_width / 2, 2, time_str, 0xffffff, 0x0, true, false);
    
    let mut i = 0u32;

    loop {
        mouse.handle_mouse_interrupt();
        mouse.draw_mouse();

        if i == 1000 {
    
            // Mostrar hora en toolbar
            let datetime = DateTime;
            let datetime_str = datetime.format();
            let time_str = unsafe { core::str::from_utf8_unchecked(&datetime_str) };

            let time_width = time_str.len() as u32 * 8;

            Graphics::draw_rect(Graphics::width() / 2 - time_width / 2, 0, time_width, 20, 0x0);
            Graphics::draw_string(Graphics::width() / 2 - time_width / 2, 2, time_str, 0xffffff, 0x0, true, false);

            i = 0;
        }

        let mut s: &str;

        match mouse.mouse.buttons {
            0 => s = "0",
            1 => s = "1",
            2 => s = "2",
            _ => s = "o",
        }

        Graphics::draw_rect(0, 20, 100, 20, 0x0);
        Graphics::draw_string(0, 20, s, 0xffffff, 0x0, false, false);
        i += 1;

        Graphics::draw_rect(0,40,100,40,0x0);
        let byte: u8 = mouse.keyboard.last_key as u8;
        Graphics::draw_char(0,40,byte, 0xffffff, 0x0, false, false);

        //mouse.wait_data();
        //if Keyboard::was_c_pressed() {
        //    Graphics::draw_rect(0,40,100,20,0x0);
        //}

        //if Keyboard::was_c_pressed() {
        //    Graphics::draw_rect(0, 0, 1920, 1080, 0xffffff);
        //    unsafe { wait_ms(5000); }
        //}
    }
 
}

#[lang = "eh_personality"] extern "C" fn eh_personality() {}
#[panic_handler] fn panic(_info: &core::panic::PanicInfo) -> ! { loop {} }
