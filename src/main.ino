#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <MD_DS3231.h>

#define SHOW_TIME 0
#define SHOW_LAMP_STATUS 1

LiquidCrystal_I2C lcd(0x27, 20, 4);

int screen_refresh = 1;

volatile int displayState;
volatile int incrementor;
volatile int dr2;
volatile int dr3;

const int changeStatePin = 2;


class Displays {

    public:
        char screen[4][21];

        void renderScreen() {
            lcd.clear();
            for (int i = 0; i <= 3; ++i) {
                lcd.setCursor(0, i);
                lcd.print(this->screen[i]);
            }
        }

        void screenWelcome()
        {
            sprintf(this->screen[0], "+------------------+");
            sprintf(this->screen[1], "|   Turtle clock   |");
            sprintf(this->screen[2], "| starting! Tsss!. |");
            sprintf(this->screen[3], "+------------------+");
            this->renderScreen();
        }

        void screenDateTime()
        {
            RTC.readTime();
            sprintf(screen[0], "%4d-%02d-%02d %02d:%02d:%02d", RTC.yyyy, RTC.mm, RTC.dd, RTC.h, RTC.m, RTC.s);
            sprintf(screen[1], "Zone 1: %2d%cC", 24, (char)223);
            sprintf(screen[2], "Zone 2: ----");
            sprintf(screen[3], "Inc: %d 2: %2d 3: %2d", incrementor, dr2, dr3);
            this->renderScreen();
        }

        void screenLampStatus()
        {
            lcd.clear();
            sprintf(screen[0], "Lapm 1 Off");
            sprintf(screen[1], "Lamp 2 Off");
            sprintf(screen[2], "Lamp 3 Off");
            sprintf(screen[3], "Lamp 4 Off");

            this->renderScreen();
        }

    typedef void (Displays::*GeneralFunction) ();

    static const GeneralFunction doActionsArray [3];

};


const Displays::GeneralFunction Displays::doActionsArray [3] =
{
    &Displays::screenWelcome, 
    &Displays::screenDateTime, 
    &Displays::screenLampStatus, 
};

Displays turtleScreen;

void setup()
{
    displayState = 0;

    pinMode(2, INPUT);
    pinMode(3, INPUT);
    attachInterrupt(digitalPinToInterrupt(3), changeState_ISR, RISING);

    lcd.begin(20, 4);
    lcd.clear();
    lcd.backlight();

    Displays::GeneralFunction f = Displays::doActionsArray [displayState++];
    (turtleScreen.*f) ();
    delay(1000);
}

void loop()
{
    int l_margin = 1;
    int r_margin = 2;
    if (screen_refresh) {
        Displays::GeneralFunction f = Displays::doActionsArray [displayState];
        (turtleScreen.*f) ();
        screen_refresh = 0;
    }

    if (incrementor == 1 && displayState < r_margin) {
        displayState += incrementor;
        screen_refresh = 1;
    } else if (incrementor == -1 && displayState > l_margin) {
        displayState += incrementor;
        screen_refresh = 1;
    }
    incrementor = 0;

    if ((displayState < 1) || (displayState > 2))
    {
        displayState = 1;
        screen_refresh = 1;
    }
}

void changeState_ISR()
{
    int rotary_pin_B;
    rotary_pin_B = digitalRead(2);

    if (rotary_pin_B == 0) {
        incrementor = 1;
    } else {
        incrementor = -1;
    }
}
