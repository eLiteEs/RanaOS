// rust/src/graphics.rs

extern "C" { // Graphics functions
    pub fn vgraphics_put_pixel(x: u32, y: u32, color: u32);
    pub fn vgraphics_draw_char(x: u32, y: u32, c: u8, fg: u32, bg: u32, bold: bool, underline: bool);
    pub fn vgraphics_clear_screen();
    pub fn vgraphics_get_width() -> u32;
    pub fn vgraphics_get_height() -> u32;
    pub fn vgraphics_put_image(dx: u32, y: u32, img_data: *const i8);
    pub fn vgraphics_get_pixel(x: u32, y: u32) -> u32;
}

pub struct Graphics;

impl Graphics {
    pub fn put_pixel(x: u32, y: u32, color: u32) {
        unsafe { vgraphics_put_pixel(x, y, color) }
    }

    pub fn draw_char(x: u32, y: u32, c: u8, fg: u32, bg: u32, bold: bool, underline: bool) {
        unsafe { vgraphics_draw_char(x, y, c, fg, bg, bold, underline) }
    }

    pub fn clear_screen() {
        unsafe { vgraphics_clear_screen() }
    }

    pub fn width() -> u32 {
        unsafe { vgraphics_get_width() }
    }

    pub fn height() -> u32 {
        unsafe { vgraphics_get_height() }
    }

    pub fn draw_string(x: u32, y: u32, s: &str, fg: u32, bg: u32, bold: bool, underline: bool) {
        for (i, c) in s.chars().enumerate() {
            if c == '\n' {
                continue; // Skip newlines
            }
            if i >= 128 { // Limit to 128 characters
                break;
            }
            unsafe { vgraphics_draw_char(x + i as u32 * 8, y, c as u8, fg, bg, bold, underline); }
        }
    }

    pub fn draw_rect(x: u32, y: u32, width: u32, height: u32, color: u32) {
        for i in 0..width {
            for j in 0..height {
                Self::put_pixel(x + i, y + j, color);
            }
        }
    }

    pub fn put_image(dx: u32, dy: u32, img_data: *const i8) {
        unsafe { vgraphics_put_image(dx, dy, img_data); }
    }

    pub fn get_pixel(x: u32, y: u32) -> u32 {
        unsafe { vgraphics_get_pixel(x, y) }
    }
}
