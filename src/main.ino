#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <MD_DS3231.h>

#define L_MENU_MARGIN 1
#define R_MENU_MARGIN 3

#define LCD_LIGHT_TIMEOUT 10000

uint8_t uparrow[8] = {0x4, 0xe, 0x15, 0x4, 0x4, 0x4, 0x0};
uint8_t downarrow[8] = {0x4, 0x4, 0x4, 0x15, 0xe, 0x4, 0x0};

LiquidCrystal_I2C lcd(0x27, 20, 4);

volatile bool screen_refresh = false;

volatile int8_t displayState;
volatile int incrementor;
volatile int dr2;
volatile int dr3;

const int changeStatePin = 2;


class Displays {

    private:
        bool backlightIsOn;
        int32_t backlightOffTime;
        unsigned long previousMillis, currentMillis;

    public:
        char screen[4][21];

        void renderScreen() {
            lcd.clear();
            this->backlightOn();
            
            for (int i = 0; i <= 3; ++i) {
                lcd.setCursor(0, i);
                lcd.print(this->screen[i]);
            }
        }

        void backlightOn()
        {
            lcd.backlight();
            this->backlightIsOn = true;
            this->previousMillis = millis();
        }

        void backlightCheck()
        {
            //if (this->backlightIsOn && this->backlightOffTime < millis())
            this->currentMillis = millis();
            if (this->backlightIsOn && 
                ((unsigned long)(this->currentMillis - this->previousMillis) >= LCD_LIGHT_TIMEOUT))
            {
                lcd.noBacklight();
                this->backlightIsOn = false;
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
            sprintf(screen[0], "Lapm 1 Off");
            sprintf(screen[1], "Lamp 2 Off");
            sprintf(screen[2], "Lamp 3 Off");
            sprintf(screen[3], "Lamp 4 Off");

            this->renderScreen();
        }

        void screenNextActions()
        {
            sprintf(screen[0], "L1 %c%02d:%02d L5 %c%02d:%02d", (char)1, 7, 0, (char)1, 7, 0);
            sprintf(screen[1], "L2 %c%02d:%02d L6 %c%02d:%02d", (char)2, 20, 30, (char)2, 20, 30);
            sprintf(screen[2], "L3 %c%02d:%02d", (char)223, 7, 0);
            sprintf(screen[3], "L4 %c%02d:%02d", (char)223, 7, 0);
            
            this->renderScreen();
        }

    typedef void (Displays::*GeneralFunction) ();

    static const GeneralFunction doActionsArray [4];

};


const Displays::GeneralFunction Displays::doActionsArray [4] =
{
    &Displays::screenWelcome,
    &Displays::screenDateTime,
    &Displays::screenLampStatus,
    &Displays::screenNextActions,
};

Displays turtleScreen;

void setup()
{
    lcd.createChar(1, uparrow);
    lcd.createChar(2, downarrow);
    displayState = 0;

    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    attachInterrupt(digitalPinToInterrupt(3), changeState_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(2), pushButton_ISR, RISING);
    

    lcd.begin(20, 4);

    Displays::GeneralFunction f = Displays::doActionsArray [displayState++];
    (turtleScreen.*f) ();
    screen_refresh = true;
    delay(1000);
}

void loop()
{
    if (screen_refresh) {
        Displays::GeneralFunction f = Displays::doActionsArray [displayState];
        (turtleScreen.*f) ();
        screen_refresh = false;
    }

    turtleScreen.backlightCheck();
}

void changeState_ISR()
{
    int rotary_pin_B;
    rotary_pin_B = digitalRead(4);

    if (rotary_pin_B == 1 && (displayState < R_MENU_MARGIN)) {
        displayState++;
        screen_refresh = true;
    } else if (rotary_pin_B == 0 && (displayState > L_MENU_MARGIN)) {
        displayState--;
        screen_refresh = true;
    }
}

void pushButton_ISR()
{
    screen_refresh = true;
}