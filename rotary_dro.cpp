#include <Encoder.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder x_pos(2, 4);
Encoder y_pos(3, 5);

// set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

const byte ROWS = 3;  // four rows
const byte COLS = 4;  // four columns
// define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
    {'r', 'y', 'x', '_'},
    {'v', '^', '>', '<'},
    {'Y', 'b', 'a', 'X'},
};
static byte colPins[COLS] = {6, 7, 8, 9};
static byte rowPins[ROWS] = {10, 11, 12};

// initialize an instance of class NewKeypad
static Keypad pressed_keypad =
    Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

static bool refresh_display = false;
ISR(TIMER1_COMPA_vect) {  // timer1 interrupt 1Hz toggles pin 13 (LED)
    refresh_display = true;
}

void setupTimer() {
    // initialize timer1

    noInterrupts();  // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 1250;             // compare match register 16MHz/256/25Hz
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
    interrupts();             // enable all interrupts
}

void setup() {
    setupTimer();
    Serial.begin(115200);
    delay(50);
    Serial.println("Starting");
    lcd.init();  // initialize the lcd

    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Starting");
}

long const ppr = 1600;
double const axle_circumference = (5.5 / 2.0) * 2 * 3.14159265;
double const gear_ratio = 4.0;
double const x_factor = (double)ppr / axle_circumference;
double const y_factor = (double)ppr / axle_circumference;

static unsigned long const MAX_COORDINATE_SYSTEMS = 5;

struct Axis {
    long position;
    long backlash;
};
struct CoordinateSystem {
    Axis x;
    Axis y;
};

static CoordinateSystem coordinate_systems[MAX_COORDINATE_SYSTEMS] = {};
static unsigned long coordinate_system_index = 0;

static long x_encoder_position = 0;
static long y_encoder_position = 0;

void loop() {
    static bool first_loop = true;
    bool x_value_changed = false;
    bool y_value_changed = false;

    if (first_loop) {
        x_value_changed = true;
        y_value_changed = true;
        first_loop = false;
    }

    char const pressed_key = pressed_keypad.getKey();

    // Increase layer of coordinate system
    if (pressed_key == 'a') {
        if ((coordinate_system_index + 1) < MAX_COORDINATE_SYSTEMS) {
            ++coordinate_system_index;
            x_value_changed = true;
            y_value_changed = true;
        }
    }

    // Decrease layer of coordinate system
    if (pressed_key == 'b') {
        if (coordinate_system_index != 0) {
            --coordinate_system_index;
            x_value_changed = true;
            y_value_changed = true;
        }
    }

    Axis& x = coordinate_systems[coordinate_system_index].x;
    Axis& y = coordinate_systems[coordinate_system_index].y;

    {
        const long encoder_position = x_pos.read();

        if (encoder_position != x_encoder_position) {
            x_encoder_position = encoder_position;
            x_value_changed = true;
        }
    }

    {
        const long encoder_position = y_pos.read();

        if (encoder_position != y_encoder_position) {
            y_encoder_position = encoder_position;
            y_value_changed = true;
        }
    }

    // Zero the x axis
    if (pressed_key == 'x') {
        x.position = -1 * x_encoder_position;
        x_value_changed = true;
    }

    // Zero the y axis
    if (pressed_key == 'y') {
        y.position = -1 * y_encoder_position;
        y_value_changed = true;
    }

    // Half the x axis
    if (pressed_key == 'X') {
        x.position -= (x.position + x_encoder_position) / 2;
        x_value_changed = true;
    }

    // Half the y axis
    if (pressed_key == 'Y') {
        y.position -= (y.position + y_encoder_position) / 2;
        y_value_changed = true;
    }

    if (refresh_display && (x_value_changed || y_value_changed)) {
        const long x_with_offset = (x.position + x_encoder_position);
        const long y_with_offset = (y.position + y_encoder_position);
        const double x_calculated = (double)(x_with_offset) / x_factor;
        const double y_calculated = (double)(y_with_offset) / y_factor;

        Serial.print("Refresh with ");
        Serial.print(x_calculated);
        Serial.print(", ");
        Serial.print(y_calculated);
        Serial.println();

        if (x_value_changed) {
            lcd.setCursor(0, 0);
            lcd.print("                ");
            lcd.setCursor(0, 0);
            lcd.print("x: ");
            lcd.print(x_calculated);
            lcd.print(" mm : ");
            lcd.print(coordinate_system_index);
        }

        if (y_value_changed) {
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print("y: ");
            lcd.print(y_calculated);
            lcd.print(" mm");
        }

        refresh_display = false;
    }

    if (pressed_key) {
        lcd.setCursor(15, 1);
        lcd.print(pressed_key);

        Serial.print("Button: ");
        Serial.print(pressed_key);
        Serial.println();
    }
}
