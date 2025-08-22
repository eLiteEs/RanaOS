// rust/src/lib.rs
#![no_std]
#![no_main]
#![feature(lang_items)]

// External C functions from your kernel
extern "C" {
    // Graphics functions
    fn vgraphics_put_pixel(x: u32, y: u32, color: u32);
    fn vgraphics_draw_char(x: u32, y: u32, c: u8, fg: u32, bg: u32, bold: bool);
    fn vgraphics_clear_screen();
    fn vgraphics_get_width() -> u32;
    fn vgraphics_get_height() -> u32;
    // Date and time functions
    fn get_second() -> u32;
    fn get_minute() -> u32;
    fn get_hour() -> u32;
    fn get_day() -> u32;
    fn get_month() -> u32;
    fn get_year() -> u32;

    // Wait functions
    fn wait_ms(ms: u32);

    // I/O functions
    fn outb(port: u16, value: u8);
    fn inb(port: u16) -> u8;

    // IRQ functions
    fn enable_irq(irq: u8);
}

// Public Rust interface
pub struct Graphics;
pub struct DateTime;

impl Graphics {
    pub fn put_pixel(x: u32, y: u32, color: u32) {
        unsafe { vgraphics_put_pixel(x, y, color) }
    }

    pub fn draw_char(x: u32, y: u32, c: u8, fg: u32, bg: u32, bold: bool) {
        unsafe { vgraphics_draw_char(x, y, c, fg, bg, bold) }
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

    pub fn draw_string(x: u32, y: u32, s: &str, fg: u32, bg: u32, bold: bool) {
        for (i, c) in s.chars().enumerate() {
            if c == '\n' {
                continue; // Skip newlines
            }
            if i >= 128 { // Limit to 128 characters
                break;
            }
            unsafe { vgraphics_draw_char(x + i as u32 * 8, y, c as u8, fg, bg, bold); }
        }
    }

    pub fn draw_rect(x: u32, y: u32, width: u32, height: u32, color: u32) {
        for i in 0..width {
            for j in 0..height {
                Self::put_pixel(x + i, y + j, color);
            }
        }
    }
}
impl DateTime {
    pub fn seconds() -> u32 {
        return unsafe { get_second() };
    }

    pub fn minutes() -> u32 {
        return unsafe { get_minute() };
    }

    pub fn hours() -> u32 {
        return unsafe { get_hour() };
    }

    pub fn day() -> u32 {
        return unsafe { get_day() };
    }

    pub fn month() -> u32 {
        return unsafe { get_month() };
    }

    pub fn year() -> u32 {
        return unsafe { get_year() };
    }
    
    pub fn format(&self) -> [u8; 21] {
        let mut buf = [b' '; 21];
    
        let write_two = |buf: &mut [u8], pos: usize, n: u32| {
            buf[pos] = b'0' + ((n / 10) % 10) as u8;
            buf[pos + 1] = b'0' + (n % 10) as u8;
        };
    
        write_two(&mut buf, 0, DateTime::hours() + 2);
        buf[2] = b':';
        write_two(&mut buf, 3, DateTime::minutes());
        buf[5] = b':';
        write_two(&mut buf, 6, DateTime::seconds());
        buf[8] = b' ';
        buf[9] = b'-';
        buf[10] = b' ';
        write_two(&mut buf, 11, DateTime::day());
        buf[13] = b'/';
        write_two(&mut buf, 14, DateTime::month());
        buf[16] = b'/';
    
        let year = DateTime::year(); // full year
        buf[17] = b'2';
        buf[18] = b'0';
        buf[19] = b'0' + ((year / 10) % 10) as u8;
        buf[20] = b'0' + (year % 10) as u8;
    
        buf
    }
}

// Estructura para el estado del ratón
pub struct Mouse {
    x: i32,
    y: i32,
    buttons: u8,
    width: u32,
    height: u32
}

// Registros PS/2
const PS2_DATA: u16 = 0x60;
const PS2_CMD: u16 = 0x64;

impl Mouse {
    pub fn new(screen_width: u32, screen_height: u32) -> Self {
        Self {
            x: (screen_width / 2) as i32,
            y: (screen_height / 2) as i32,
            buttons: 0,
            width: screen_width,
            height: screen_height
        }
    }

    // Inicialización del ratón
    pub unsafe fn init(&mut self) {
        // Habilitar dispositivo auxiliar (ratón)
        self.wait_cmd();
        outb(PS2_CMD, 0xA8);
        
        // Habilitar interrupciones
        self.wait_cmd();
        outb(PS2_CMD, 0x20);
        self.wait_data();
        let status = inb(PS2_DATA) | 0x02;
        self.wait_cmd();
        outb(PS2_CMD, 0x60);
        self.wait_data();
        outb(PS2_DATA, status);
        
        // Configurar ratón
        self.write(0xF6);  // Usar configuraciones por defecto
        self.read();       // ACK
        self.write(0xF4);  // Habilitar streaming
        self.read();       // ACK
    }

    // Esperar a que el controlador esté listo
    fn wait_cmd(&self) {
        while (unsafe { inb(PS2_CMD) } & 0x02) != 0 {}
    }

    fn wait_data(&self) {
        while (unsafe { inb(PS2_CMD) } & 0x01) == 0 {}
    }

    // Enviar comando al ratón
    fn write(&mut self, value: u8) {
        self.wait_cmd();
        unsafe { outb(PS2_CMD, 0xD4); }
        self.wait_cmd();
        unsafe { outb(PS2_DATA, value); }
    }

    // Leer dato del ratón
    fn read(&mut self) -> u8 {
        self.wait_data();
        unsafe { inb(PS2_DATA) }
    }

    // Manejar interrupción del ratón
    pub fn handle_interrupt(&mut self) {
        static mut CYCLE: u8 = 0;
        static mut PACKET: [u8; 3] = [0; 3];

        unsafe {
            match CYCLE {
                0 => {
                    PACKET[0] = self.read();
                    if PACKET[0] & 0x08 != 0 { CYCLE = 1; }
                },
                1 => {
                    PACKET[1] = self.read();
                    CYCLE = 2;
                },
                2 => {
                    PACKET[2] = self.read();
                    
                    // Actualizar posición
                    self.buttons = PACKET[0] & 0x07;
                    self.x += (PACKET[1] as i8) as i32;
                    self.y -= (PACKET[2] as i8) as i32;
                    
                    // Limitar a los bordes de la pantalla
                    self.x = self.x.max(0).min((self.width - 1) as i32);
                    self.y = self.y.max(0).min((self.height - 1) as i32);
                    
                    CYCLE = 0;
                },
                _ => CYCLE = 0,
            }
        }
    }

    // Dibujar cursor (simple)
    pub fn draw(&self) {
        Graphics::draw_rect(self.x as u32, self.y as u32, 10, 10, 0xFFFFFF);
    }
}

#[no_mangle]
pub extern "C" fn start_so() {
    Graphics::clear_screen();
    
    // Draw background
    Graphics::draw_rect(0, 0, Graphics::width(), Graphics::height(), 0x42f590);

    // Initialize mouse
    let width = Graphics::width();
    let height = Graphics::height();
    
    let mut mouse = Mouse::new(width, height);
    unsafe { mouse.init() };
    unsafe {
        // (Aquí debes implementar tu configuración de IDT)
        enable_irq(12);  // Habilitar IRQ del ratón
    }

    // Draw toolbar
    Graphics::draw_rect(0, 0, Graphics::width(), 20, 0x000000);
    Graphics::draw_string(2, 2, "RanaOS beta 2 | Press \'r\' for opening launch dialog", 0xffffff, 0x000000, false);

    // Display datetime in toolbar (right-aligned)
    let datetime = DateTime;
    let datetime_str = datetime.format();
    let time_str = unsafe { core::str::from_utf8_unchecked(&datetime_str) };
    
    let time_width = time_str.len() as u32 * 8;

    Graphics::draw_string(Graphics::width() / 2 - time_width / 2, 2, time_str, 0xffffff, 0x0, true);
    
    loop {
        // Display datetime in toolbar (middle-aligned)
        let datetime = DateTime;
        let datetime_str = datetime.format();
        let time_str = unsafe { core::str::from_utf8_unchecked(&datetime_str) };
        
        let time_width = time_str.len() as u32 * 8;

        Graphics::draw_rect(Graphics::width() / 2 - time_width / 2, 0, time_width, 20, 0x0);

        Graphics::draw_string(Graphics::width() / 2 - time_width / 2, 2, time_str, 0xffffff, 0x0, true);

        unsafe  { wait_ms(250); }       
    }
}

#[lang = "eh_personality"] extern "C" fn eh_personality() {}
#[panic_handler] fn panic(_info: &core::panic::PanicInfo) -> ! { loop {} }
