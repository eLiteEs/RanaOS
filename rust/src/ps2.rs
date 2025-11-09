use Graphics;

extern "C" {
    pub fn outb(port: u16, value: u8);
    pub fn inb(port: u16) -> u8;

    pub fn was_c_pressed() -> bool;
    pub fn was_key_pressed(key: char) -> bool;
    pub fn key_pressed() -> char;
}

// PS/2 Ports
const PS2_DATA: u16 = 0x60;
const PS2_CMD: u16 = 0x64;

/// Representa el estado de un ratón PS/2
#[derive(Default)]
pub struct MouseState {
    pub x: i32,
    pub y: i32,
    pub buttons: u8,  // bit0 = izquierda, bit1 = derecha, bit2 = medio
    pub scroll: i8,
}

/// Representa el estado del teclado PS/2
#[derive(Default)]
pub struct KeyboardState {
    pub last_key: char,
}

/// PS2 controller (teclado + ratón)
pub struct PS2 {
    pub mouse: MouseState,
    pub keyboard: KeyboardState,
    prev_mouse_x: i32,
    prev_mouse_y: i32,
    saved_pixel: u32,
    width: u32,
    height: u32,
}

impl PS2 {
    pub fn new(screen_width: u32, screen_height: u32) -> Self {
        Self {
            mouse: MouseState::default(),
            keyboard: KeyboardState::default(),
            prev_mouse_x: (screen_width / 2) as i32,
            prev_mouse_y: (screen_height / 2) as i32,
            saved_pixel: 0xffffff,
            width: screen_width,
            height: screen_height,
        }
    }

    /// Inicializa teclado y ratón
    pub unsafe fn init(&mut self) {
        // Habilitar ratón
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
        self.write_mouse(0xF6); // defaults
        self.read_mouse();       // ACK
        self.write_mouse(0xF4); // enable streaming
        self.read_mouse();       // ACK
    }

    fn wait_cmd(&self) {
        while (unsafe { inb(PS2_CMD) } & 0x02) != 0 {}
    }

    pub fn wait_data(&self) {
        while (unsafe { inb(PS2_CMD) } & 0x01) == 0 {}
    }

    fn write_mouse(&self, value: u8) {
        self.wait_cmd();
        unsafe { outb(PS2_CMD, 0xD4); }
        self.wait_cmd();
        unsafe { outb(PS2_DATA, value); }
    }

    fn read_mouse(&self) -> u8 {
        self.wait_data();
        unsafe { inb(PS2_DATA) }
    }

    /// Maneja interrupción del ratón
    pub fn handle_mouse_interrupt(&mut self) {
        static mut CYCLE: u8 = 0;
        static mut PACKET: [u8; 3] = [0; 3];

        unsafe {
            match CYCLE {
                0 => {
                    PACKET[0] = self.read_mouse();
                    if PACKET[0] & 0x08 != 0 { CYCLE = 1; }
                },
                1 => { PACKET[1] = self.read_mouse(); CYCLE = 2; },
                2 => {
                    PACKET[2] = self.read_mouse();
                    self.mouse.buttons = PACKET[0] & 0x07;
                    self.mouse.x += (PACKET[1] as i8) as i32;
                    self.mouse.y -= (PACKET[2] as i8) as i32;

                    // limitar a bordes de pantalla
                    self.mouse.x = self.mouse.x.max(0).min((self.width - 1) as i32);
                    self.mouse.y = self.mouse.y.max(0).min((self.height - 1) as i32);

                    CYCLE = 0;
                },
                _ => CYCLE = 0,
            }
        }
    }

    /// Maneja interrupción del teclado
    pub fn handle_keyboard_interrupt(&mut self) {
        let scancode = unsafe { inb(PS2_DATA) };
        self.keyboard.last_key = match scancode {
            0x1E => 'a', 0x30 => 'b', 0x2E => 'c', 0x20 => 'd', 0x12 => 'e',
            0x21 => 'f', 0x22 => 'g', 0x23 => 'h', 0x17 => 'i', 0x24 => 'j',
            0x25 => 'k', 0x26 => 'l', 0x32 => 'm', 0x31 => 'n', 0x18 => 'o',
            0x19 => 'p', 0x10 => 'q', 0x13 => 'r', 0x1F => 's', 0x14 => 't',
            0x16 => 'u', 0x2F => 'v', 0x11 => 'w', 0x2D => 'x', 0x15 => 'y',
            0x2C => 'z', 0x39 => ' ', 0x0E => '\x08', 0x1C => '\n', _ => '\0',
        };
    }

    /// Dibuja el cursor del ratón
    pub fn draw_mouse(&mut self) {
        self.restore_mouse_background();
        self.save_mouse_background();

        Graphics::put_pixel(self.mouse.x as u32, self.mouse.y as u32, 0x000000);

        self.prev_mouse_x = self.mouse.x;
        self.prev_mouse_y = self.mouse.y;
    }

    fn save_mouse_background(&mut self) {
        self.saved_pixel = Graphics::get_pixel(self.mouse.x as u32, self.mouse.y as u32);
    }

    fn restore_mouse_background(&self) {
        Graphics::put_pixel(self.prev_mouse_x as u32, self.prev_mouse_y as u32, self.saved_pixel);
    }

    /// Obtiene la última tecla presionada
    pub fn key_pressed(&self) -> char {
        self.keyboard.last_key
    }

    /// Obtiene botones del ratón
    pub fn mouse_buttons(&self) -> u8 {
        self.mouse.buttons
    }
}

