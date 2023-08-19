#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <EEPROM.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// pin info
int mapPin = A0; // map sensor
int solPin = 3; // boost solenoid

int lcdsda = A4; // screen data
int lcdscl = A5; // screen clear

int cycle = 5;
int select = 7;
int up = 10;
int down = 11;

// button info
bool presswaitc = 0;
bool presswaitd = 0;
bool presswaits = 0;

// boost info
float mapReading = 0;
int openStart = 0;
int openEnd = 0;
int springPress = 25; // spring pressure in bar*100, 25 = .25 BAR = 3.6 PSI
int scale = 0; // 0 = linear, 1 = exponential (2), 2 = exponential (3), 3 = exponential (4)

// screen info
int unit = 0; // 0 = bar, 1 = psi, 2 = kPa
int mode = 0; // 0 = main screen, 1 = change unit, 2 = spring pressure, 3 = change openStart, 4 = change openEnd, 5 = change scale
int soption = 0; // used by the handler for saving options
int tempunit = 0; // used for modes 2, 3, and 4 for using a different unit than the preferred
int dig1 = 0; // used for number dial saving on screen
int dig2 = 0; // used for number dial saving on screen
int dig3 = 0; // used for number dial saving on screen
int dig4 = 0; // used for number dial saving on screen
bool changed = 0;

float exzval = 0;

void setup() {
  // put your setup code here, to run once:
  TCCR2B = TCCR2B & B11111000 | B00000111; // for PWM frequency of 30.64 Hz, 
  pinMode(solPin, OUTPUT);
  pinMode(lcdsda, OUTPUT);
  pinMode(lcdscl, OUTPUT);

  pinMode(cycle, INPUT);
  pinMode(select, INPUT);
  pinMode(up, INPUT);
  pinMode(down, INPUT);

  Serial.begin(9600);
  readValues();
  if (scale != 0) {
    calculateEXZval();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  double mapReading = .66 * analogRead(mapPin) * (5.0/1023.0) - 1.06;
  analogWrite(solPin, calculateSolenoid(mapReading));

  // cycle through modes, one mode switch per press
  if (digitalRead(cycle) == HIGH && presswaitc == 0) {
    presswaitc = 1;
    modeSwitch();
    if (++mode == 6) {
      mode = 0;
    }
  }

  // reset cycle button
  if (digitalRead(cycle) == LOW) {
    presswaitc = 0;
  }

  // select button, used for changing digit spot on modes 2,3,4
  if (digitalRead(select) == HIGH && presswaits == 0) {
    presswaits = 1;
    // select button pressed
  }

  // reset  select
  if (digitalRead(select) == LOW) {
    presswaits = 0;
  }

  // directional button pressed
  if ((digitalRead(up) == HIGH || digitalRead(down) == HIGH) && presswaitd == 0) {
    presswaitd = 1;
  }

  // directional reset
  if (digitalRead(up) == LOW && digitalRead(down) == LOW) {
    presswaitd = 0;
  }
}

void screen(double mapReading) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  if (mode == 0) {
    display.setCursor(0,0);
    display.setTextSize(3);
    display.println("Boost");
    if (mapReading <= 0) {
      display.print(0);
    } else if (unit == 0) {
      display.print(mapReading);
    } else if (unit == 1){
      display.print((mapReading*14.5));
    } else {
      display.print(mapReading)*100;
    }

    if (unit == 0) {
      display.println(" BAR");
    } else if (unit == 1) {
      display.println(" PSI");
    } else {
      display.println(" kPa");
    }
  } else if (mode == 1) {
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("Preferred Unit:");
    if (soption == 0) {
      display.setTextColor(BLACK, WHITE);
    }
    display.println("BAR");
    display.setTextColor(WHITE);
    if (soption == 1) {
      display.setTextColor(BLACK, WHITE);
    }
    display.println("PSI");
    display.setTextColor(WHITE);
    if (soption == 2) {
      display.setTextColor(BLACK, WHITE);
    }
    display.println("kPa");
  } else if (mode == 5) {
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("Set Scale:");
    if (soption == 0) {
      display.setTextColor(BLACK, WHITE);
    }
    display.println("LINEAR");
    display.setTextColor(WHITE);
    if (soption == 1) {
      display.setTextColor(BLACK, WHITE);
    }
    display.println("X^2");
    display.setTextColor(WHITE);
    if (soption == 2) {
      display.setTextColor(BLACK, WHITE);
    }
    display.println("X^3");
    display.setTextColor(WHITE);
    if (soption == 3) {
      display.setTextColor(BLACK, WHITE);
    }
    display.println("X^4");
  }else if (mode == 2 || mode == 3 || mode == 4) {
    display.setCursor(0,0);
    display.setTextSize(1);
    if (mode == 2) {
      display.println("Set Spring Pressure:");
    } else if (mode == 3) {
      display.println("Set Crack Pressure:");
    } else if (mode == 4) {
      display.println("Set Max Boost:");
    }
    if (soption == 0) {
      display.setTextColor(BLACK, WHITE);
    }
    display.print(dig1);
    display.setTextColor(WHITE);
    if (tempunit == 0) {
      display.print(".");
    }
    if (soption == 1) {
      display.setTextColor(BLACK, WHITE);
    }
    display.print(dig2);
    display.setTextColor(WHITE);
    if (tempunit == 1) {
      display.print(".");
    }
    if (soption == 2) {
      display.setTextColor(BLACK, WHITE);
    }
    display.print(dig3);
    display.setTextColor(WHITE);
    if (tempunit == 1) {
      if (soption == 3) {
        display.setTextColor(BLACK, WHITE);
      }
      display.print(dig4);
      display.setTextColor(WHITE);
    }
    if (soption == 4) {
      display.setTextColor(BLACK, WHITE);
    }
    if (tempunit == 0) {
      display.print(" BAR");
    } else if (tempunit == 1) {
      display.print(" PSI");
    } else if (tempunit == 2) {
      display.print(" kPa");
    }
  }

  display.display();
}

// handle saving values, resetting values for other screens
void modeSwitch() {
  // load unit into option select
  if (mode == 0) {
    soption = unit;
  }
  else if (mode == 1) {
    // save unit
    if (soption != unit) {
      unit = soption;
      saveValues();
    }
    // load spring select
    soption = 0;
    tempunit = unit;
    int value = 0;
    setDigits(springPress, tempunit);

  } else if (mode == 2 || mode == 3 || mode == 4) {
    soption = 0;
    if (changed == 1) {
      int value = 0;
      if (tempunit == 1) {
        value = (dig1*1000+dig2*100+dig3*10+dig4)/14.5;
      } else {
        value = dig1*100+dig2*10+dig3;
      }
      if (mode == 2) {
        springPress = value;
        if (springPress > openEnd) {
          openEnd = value;
        }
      } else if (mode == 3) {
        openStart = value;
        if (openStart > openEnd) {
          openEnd = openStart;
        }
      } else if (mode == 4) {
        openEnd = value;
        if (openEnd < openStart) {
          openEnd = openStart;
        }
        if (openEnd < springPress) {
          openEnd = springPress;
        }
      }
      saveValues();
      calculateEXZval();
    }
    if (mode == 2) {
      setDigits(openStart, tempunit);
    } else if (mode == 3) {
      setDigits(openEnd, tempunit);
    } else if (mode == 4) {
      soption = scale;
    }
    changed = 0;
  } else if (mode == 5) {
    scale = soption;
    calculateEXZval();
    saveValues();
    soption = 0;
  }
}

void setDigits(int value, int unit) {
  int t;
  if (tempunit == 1) {
    t = value*14.5;
  } else {
    t = value;
  }
  // set digits
  if (tempunit == 1) {
    dig4 = t % 10;
    t = t/10;
    dig3 = t % 10;
    t = t/10;
    dig2 = t % 10;
    t = t/10;
    dig1 = t % 10;
  } else {
    dig4 = 0;
    dig3 = t % 10;
    t = t/10;
    dig2 = t % 10;
    t = t/10;
    dig1 = t % 10;
  }
}

// assign button controls to actions 
void modeControl(int button) {
  // 0 = select, 1 = up, 2 = down
  if (mode == 0) {
    // main screen
    return;
  } else if ((mode == 1 || mode == 5) && button == 0) {
    // select button does nothing on change unit and change scale screens
    return;
  } else if (mode == 1) {
    // cycle unit on unit select screen
    if (button == 1) {
      soption--;
      if (soption == -1) {
        soption = 2;
      }
    } else if (button == 2) {
      soption++;
      if (soption == 3) {
        soption = 0;
      }
    }
  } else if (mode == 5) {
    // cycle scale option
    if (button == 1) {
      soption--;
      if (soption == -1) {
        soption = 2;
      }
    } else if (button == 2) {
      soption++;
      if (soption == 4) {
        soption = 0;
      }
    }
  } else if (mode == 2 || mode == 3 || mode == 4) {
    // spring set control
    if (button == 0) {
      soption++;
      if (soption == 5) {
        soption = 0;
      }
      // 3 digit slots for bar, kPa, 4 for psi
      if (soption == 3 && tempunit != 1) {
        soption = 4;
      }
    } else {
      changed = 1;
      int dir = 0;
      if (button == 1) {
        dir++;
      } else {
        dir--;
      }
      if (soption == 0) {
        dig1+=dir;
      } else if (soption == 1) {
        dig2+=dir;
      } else if (soption == 2) {
        dig3+=dir;
      } else if (soption == 3) {
        dig4+=dir;
      } else if (soption == 4) {
        int value = 0;
        if (tempunit == 1) {
          int value = (dig1*1000+dig2*100+dig3*10+dig4)/14.5;
        } else if (tempunit+dir == 1) {
          int value = (dig1*100+dig2*10+dig3);
        }
        tempunit+=dir;
        setDigits(value, tempunit);
      }
      if (dig4 == 10) {
        dig4 = 0;
        dig3++;
      }
      if (dig3 == 10) {
        dig3 = 0;
        dig2++;
      }
      if (dig2 == 10) {
        dig2 = 0;
        dig1++;
      }
      if (tempunit == 1) {
        if (dig1 > 2 || (dig1 == 2 && dig2 == 9 && (dig3 > 0 || dig4 > 0))) {
          dig1 = 2;
          dig2 = 9;
          dig3 = 0;
          dig4 = 0;
        }
      } else {
        if (dig1 >= 2) {
          dig1 = 2;
          dig2 = 0;
          dig3 = 0;
        }
      }
    }
  }
}

int calculateSolenoid(float reading) {
  int bar = reading*100;
  if (reading <= openStart) {
    return 255;
  } else if (reading > openEnd) {
    return 0;
  } else {
    if (scale == 0) {
      int diff = openEnd - openStart;
      double increment = 100.0/diff;
      double percent = ((100.0-(bar-openStart)*increment)/100.0);
      return (percent*(springPress/bar))*255;
    } else {
      return (255-(exzval*pow(bar-openStart, scale+1)))*(springPress/bar);
    }
  }
}

void calculateEXZval() {
  if (openEnd - openStart == 0) {
    exzval = 0;
    return;
  }
  exzval = 255.0/pow(openEnd - openStart, scale+1);
}

/* EEPROM order:
  openStart
  openEnd
  unit
  scale
  springPress
*/

void saveValues() {
  if (openStart != EEPROM.read(0)) {
    EEPROM.write(0, openStart);
  }
  if (openEnd != EEPROM.read(1)) {
    EEPROM.write(1, openEnd);
  }
  if (unit != EEPROM.read(2)) {
    EEPROM.write(2, unit);
  }
  if (tempunit != EEPROM.read(3)) {
    EEPROM.write(3, tempunit);
  }
  if (scale != EEPROM.read(4)) {
    EEPROM.write(4, scale);
  }
  if (springPress != EEPROM.read(5)) {
    EEPROM.write(5, springPress);
  }
}

void readValues() {
  // values never set
  if (EEPROM.read(3) == 255) {
    // load defaults
    openStart = 25;
    openEnd = 25;
    unit = 0;
    tempunit = 0;
    scale = 0;
    springPress = 25;
    saveValues();
  } else {
    openStart = EEPROM.read(0);
    openEnd = EEPROM.read(1);
    unit = EEPROM.read(2);
    tempunit = EEPROM.read(3);
    scale = EEPROM.read(4);
    springPress = EEPROM.read(5);
  }
}