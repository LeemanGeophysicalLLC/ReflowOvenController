/*
 * Stepper Motor Driver for GF-10 Reflow Oven
 * Leeman Geophysical LLC
 * John R. Leeman
 * 
 * Controls a stepper motor to produce a given conveyor speed replacing the old
 * faulty AC tachometer system. The AC tach signal must be faked out to avoid
 * tripping the over/under speed safety. That is does with a simple voltage
 * divider outside of this system.
 * 
 * Connections:
 * Digital Pin 6 - Step Pin
 * Digital Pin 7 - Direction Pin
 * Digital Pin 12 - LCD RS
 * Digital Pin 11 - LCD EN
 * Digital Pin 5 - LCD D4
 * Digital Pin 4 - LCD D5
 * Digital Pin 3 - LCD D6
 * Digital Pin 2 - LCD D7
 * GND - Step -
 * GND - Direction -
 * GND - LCD backlight -
 * 5V - LCD backlight +
 */

#include <Encoder.h>
#include <LiquidCrystal.h>

// Math Constants
const uint16_t pulses_per_revolution = 12800;
const uint16_t belt_mm_per_revolution = 239;
const float belt_reduction = 0.555555555;

float calculate_rpm_from_speed(uint16_t speed_mm)
{
  // Calculate the RPM needed to get to the given speed
  return float(speed_mm) / belt_mm_per_revolution / belt_reduction;
}


float calculate_microseconds_per_cycle_from_rpm(float rpm)
{
  // Calculate the microseconds per cycle from the desired RPM
  float steps_per_minute = rpm * pulses_per_revolution;
  float microseconds_per_cycle = 1000000 / (steps_per_minute / 60);
  return microseconds_per_cycle;
}

// Pins
const uint8_t PIN_ENCODER_A = 2;  // Blue
const uint8_t PIN_ENCODER_B = 3;  // Yellow
const uint8_t PIN_LCD_RS = 12;  // Purple
const uint8_t PIN_LCD_EN = 11;  // Blue
const uint8_t PIN_LCD_D4 = 10;  // Green
const uint8_t PIN_LCD_D5 = 9;  // Yellow
const uint8_t PIN_LCD_D6 = 8;  // Orange
const uint8_t PIN_LCD_D7 = 7;  // Red
const uint8_t PIN_LCD_CONTRAST = 6;  // Grey 
const uint8_t PIN_STEP = 4;  // Purple
const uint8_t PIN_DIRECTION = 5;  // Orange
// Power LCD with 5V (White), GND (Black)
// Encoder also has GND (Black)

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
Encoder myEnc(PIN_ENCODER_B, PIN_ENCODER_A); // Encoder backwards acting? Switch pins A and B!

// Globals
int16_t speed_mm_per_minute = 0;
float rpm_setpoint = calculate_rpm_from_speed(speed_mm_per_minute);
float microseconds_per_cycle = calculate_microseconds_per_cycle_from_rpm(rpm_setpoint);

void setup()
{
  // Set initial speed
  myEnc.write(152);

  // Pin setup and contrast setting
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIRECTION, OUTPUT);
  pinMode(PIN_LCD_CONTRAST, OUTPUT);
  analogWrite(PIN_LCD_CONTRAST, 30);

  // Welcome and screen setup
  lcd.begin(16, 2);
  lcd.setCursor(2, 0);
  lcd.print("Reflow Oven");
  lcd.setCursor(0, 1);
  lcd.print("Conveyor Control");
  delay(2500);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Leeman");
  lcd.setCursor(0, 1);
  lcd.print("Geophysical LLC");
  delay(2500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SPEED:    mm/min");
  lcd.setCursor(0, 1);
  lcd.print("RPM:");

  // Set the motor direction
  digitalWrite(PIN_DIRECTION, HIGH);

  // Set the screen values
  update_screen_values(rpm_setpoint, speed_mm_per_minute);

}

void update_screen_values(float rpm, uint16_t mm_per_minute)
{
  // Write the screen values
  lcd.setCursor(6, 0);
  lcd.print("          ");
  lcd.setCursor(4, 1);
  lcd.print("            ");
  lcd.setCursor(6, 0);
  lcd.print(speed_mm_per_minute);
  lcd.setCursor(4, 1);
  lcd.print(rpm_setpoint);
}

void loop()
{
  // Check for an updated setting
  int16_t new_speed = myEnc.read();
  if (new_speed != speed_mm_per_minute)
  {
    if (new_speed < 0)
    {
      myEnc.write(0);
      new_speed = 0;
    }
    speed_mm_per_minute = new_speed;
    
    // Do the calculation/control
    rpm_setpoint = calculate_rpm_from_speed(speed_mm_per_minute);
    microseconds_per_cycle = calculate_microseconds_per_cycle_from_rpm(rpm_setpoint);

    // Update the screen
    update_screen_values(rpm_setpoint, speed_mm_per_minute);
  }

  if (rpm_setpoint > 0)  // Protected so we don't go fast when speed is set to zero!
  {
    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(microseconds_per_cycle / 2);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(microseconds_per_cycle / 2);
  }
}
 
