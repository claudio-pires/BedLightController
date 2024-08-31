#include "extras/PwmPin.h"  // NEW! Include this HomeSpan "extra" to create LED-compatible PWM signals on one or more pinsn
#include "UTILS.h"

// Redefine time for button actions
#define LONGPULSE 1200
#define MINIMUMPULSE 10
#define SHORTPULSE 300

// Action definitions for pushbuttons
#define NOTHING -1
#define OFF 0
#define ON 1
#define TOGGLE 2

#define MAX_PUSHBUTTONS 3

// =====================================================================================
// BUTTON_ACTION: Class for manage button action configuration
// =====================================================================================
struct BUTTON_ACTION {

    int controlPin;                                    // pin associated for the input pin that control this device by hardware
    int actionOnLong, actionOnSingle, actionOnDouble;  // action type for each type of button press

    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // controlPin       reference for the input button pin assigned for the class instance
    // actionOnLong     action to be done on long press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnSingle   action to be done on normal press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnDouble   action to be done on quick double click press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // -----------------------------------------------------------------------------------
    BUTTON_ACTION(int controlPin, int actionOnLong, int actionOnSingle, int actionOnDouble) {

        this->controlPin = controlPin;
        this->actionOnLong = actionOnLong;
        this->actionOnSingle = actionOnSingle;
        this->actionOnDouble = actionOnDouble;
    }
};

// ---------------------------------------------------------------------------------------
// Action2Name() method
//
// returns action name
// action         Action id
// ---------------------------------------------------------------------------------------
const char *Action2Name(int action) {
    return ((action == ON ? "Turns ON" : (action == OFF ? "Turns OFF" : (action == TOGGLE ? "Toggles ON or OFF" : "Does nothing"))));

} 


// =====================================================================================
// DEV_LED: Class for manage on/off light type devices
// =====================================================================================
struct DEV_LED : Service::LightBulb {  // ON/OFF LED

    int ledPin;                                 // pin number defined for this LED
    SpanCharacteristic *power;                  // reference to the On Characteristic
    int actions = 0;                            // pushbutton counter
    BUTTON_ACTION *action[MAX_PUSHBUTTONS];     // rules for pushbuttons
    char *accessoryName;                        // accessory name just for login purpose

    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // ledPin           reference for the output pin assigned for the class instance
    // -----------------------------------------------------------------------------------
    DEV_LED(int ledPin, char *name): Service::LightBulb() {
        this->actions = 0;  // Initialize as no actions
        power = new Characteristic::On();
        power->setVal(0);
        this->ledPin = ledPin;
        this->accessoryName = name;
        pinMode(ledPin, OUTPUT);  // Only required for on/off pins
        //digitalWrite(ledPin, 1); // As quick as possible to set up to avoid flicking
        update();
    }

    // -----------------------------------------------------------------------------------
    // setActionsOnSpanButton() method
    //
    // Attachs action configuration to a specific pin
    //
    // controlPin       reference for the input button pin assigned for the class instance
    // actionOnLong     action to be done on long press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnSingle   action to be done on normal press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnDouble   action to be done on quick double click press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // -----------------------------------------------------------------------------------
    boolean setActionsOnSpanButton(int controlPin, int actionOnLong, int actionOnSingle, int actionOnDouble) {

        if (actions == MAX_PUSHBUTTONS) {
            return false;                   // Only MAX_PUSHBUTTONS actions supported per accesory
        }
        WEBLOG("Configuring ON/OFF lamp for accessory %s[Pin#%d] with control pushbutton on pin %d and index #%d", accessoryName, ledPin, controlPin, actions);
        WEBLOG("      %s on normal click", Action2Name(actionOnSingle));
        WEBLOG("      %s on double click", Action2Name(actionOnDouble));
        WEBLOG("      %s on long press", Action2Name(actionOnLong));
        new SpanButton(controlPin, LONGPULSE, MINIMUMPULSE, SHORTPULSE);
        this->action[actions] = new BUTTON_ACTION(controlPin, actionOnLong, actionOnSingle, actionOnDouble);
        this->actions++;
        return true;
    }

    // -----------------------------------------------------------------------------------
    // update() method
    //
    // updates hardware pin status.
    // Note that since library manages normally closed relay Ive changed to 1 - val()
    // -----------------------------------------------------------------------------------
    boolean update() {
        digitalWrite(ledPin, 1 - power->getNewVal());
        return true;  // returns true
    }

    // -----------------------------------------------------------------------------------
    // button() method
    //
    // handles clic on button. Library pass button pin id and press type so its responsability
    // of the handler to decide what to do with the button an press type
    // -----------------------------------------------------------------------------------
    void button(int pin, int pressType) override {

        LOG2("Accessory %s Pin #%d button handler got button press on pin %d with type %s\n", accessoryName, ledPin, pin, pressType == SpanButton::LONG ? "LONG" : (pressType == SpanButton::SINGLE ? "SINGLE" : (pressType == SpanButton::DOUBLE ? "DOUBLE" : "???")));
        for (int i = 0; i < this->actions; i++) {
            LOG2("   Pushbutton index #%d ", i);
            if (pin == action[i]->controlPin) {
                LOG2("configured for pin #%d ", pin);
                int a = ((pressType == SpanButton::SINGLE) ? action[i]->actionOnSingle : ((pressType == SpanButton::DOUBLE) ? action[i]->actionOnDouble : ((pressType == SpanButton::LONG) ? action[i]->actionOnLong : NOTHING)));
                LOG2("with action: #%s\n", Action2Name(a));
                if(a == NOTHING) {
                    LOG2("      Nothing to do\n");
                } else {
                    if(a == OFF) {
                        power->setVal(0);
                        LOG2("      Accessory going OFF\n");
                    } else {
                        if(a == TOGGLE) {
                            power->setVal(1 - power->getVal());
                            LOG2("      Accessory going %s\n", power->getVal() == 0 ? "ON" : "OFF");
                        } else {
                            power->setVal(1);
                            LOG2("      Accessory going ON\n");
                        }
                    }
                    update();
                    LOG2("      Updating...\n");
                }
                LOG2("   Exit handler\n");
                return;
            } else {
                LOG2("not configured for pin #%d \n", pin);
            }
        }
        LOG2("   Exit handler\n");
    }
};

// =====================================================================================
// DEV_DimmableLED: Class for manage dimmable light type devices (PWM based)
// =====================================================================================
struct DEV_DimmableLED : Service::LightBulb {  // Dimmable LED

    LedPin *ledPin;                             // NEW! Create reference to LED Pin instantiated below
    SpanCharacteristic *power;                  // reference to the On Characteristic
    SpanCharacteristic *level;                  // NEW! Create a reference to the Brightness Characteristic instantiated below
    int actions = 0;                            // pushbutton counter
    BUTTON_ACTION *action[MAX_PUSHBUTTONS];     // rules for pushbuttons
    char *accessoryName;                        // accessory name just for login purpose


    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // pin              reference for the output pin assigned for the class instance
    // -----------------------------------------------------------------------------------
    DEV_DimmableLED(int pin, char *name) : Service::LightBulb() {  // constructor() method
        power = new Characteristic::On();
        level = new Characteristic::Brightness(50);  // NEW! Instantiate the Brightness Characteristic with an initial value of 50% (same as we did in Example 4)
        level->setRange(5, 100, 1);                  // NEW! This sets the range of the Brightness to be from a min of 5%, to a max of 100%, in steps of 1% (different from Example 4 values)
        this->ledPin = new LedPin(pin);              // NEW! Configures a PWM LED for output to the specified pin.  Note pinMode() does NOT need to be called in advance
        this->accessoryName = name;
        update();
    }  // end constructor

    // -----------------------------------------------------------------------------------
    // update() method
    //
    // updates hardware pin status.
    // -----------------------------------------------------------------------------------
    boolean update() {  // update() method
        ledPin->set(power->getNewVal() * level->getNewVal());
        return true;  // return true
    }

    // -----------------------------------------------------------------------------------
    // setActionsOnSpanButton() method
    //
    // Attachs action configuration to a specific pin
    //
    // controlPin       reference for the input button pin assigned for the class instance
    // actionOnLong     action to be done on long press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnSingle   action to be done on normal press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnDouble   action to be done on quick double click press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // -----------------------------------------------------------------------------------
    boolean setActionsOnSpanButton(int controlPin, int actionOnLong, int actionOnSingle, int actionOnDouble) {

        if (actions == MAX_PUSHBUTTONS) {
            return false;                   // Only MAX_PUSHBUTTONS actions supported per accesory
        }
        WEBLOG("Configuring ON/OFF dimmable lamp for accessory %s[Pin#%d] with control pushbutton on pin %d and index #%d", accessoryName, ledPin->getPin(), controlPin, actions);
        WEBLOG("      %s on normal click", Action2Name(actionOnSingle));
        WEBLOG("      %s on double click", Action2Name(actionOnDouble));
        WEBLOG("      %s on long press", Action2Name(actionOnLong));
        new SpanButton(controlPin, LONGPULSE, MINIMUMPULSE, SHORTPULSE);
        this->action[actions] = new BUTTON_ACTION(controlPin, actionOnLong, actionOnSingle, actionOnDouble);
        actions++;
        return true;
    }

    // -----------------------------------------------------------------------------------
    // button() method
    //
    // handles clic on button. Library pass button pin id and press type so its responsability
    // of the handler to decide what to do with the button an press type
    // TODO: review power and level set values
    // -----------------------------------------------------------------------------------
    void button(int pin, int pressType) override {

        LOG2("Accessory %s Pin #%d button handler got button press on pin %d with type %s\n", accessoryName, ledPin, pin, pressType == SpanButton::LONG ? "LONG" : (pressType == SpanButton::SINGLE ? "SINGLE" : (pressType == SpanButton::DOUBLE ? "DOUBLE" : "???")));
        for (int i = 0; i < this->actions; i++) {
            LOG2("   Pushbutton index #%d ", i);
            if (pin == action[i]->controlPin) {
                LOG2("configured for pin #%d ", pin);
                int a = ((pressType == SpanButton::SINGLE) ? action[i]->actionOnSingle : ((pressType == SpanButton::DOUBLE) ? action[i]->actionOnDouble : ((pressType == SpanButton::LONG) ? action[i]->actionOnLong : NOTHING)));
                LOG2("with action: #%s\n", Action2Name(a));
                if(a == NOTHING) {
                    LOG2("      Nothing to do\n");
                } else {
                    if(a == OFF) {
                        power->setVal(0);
                        LOG2("      Accessory going OFF\n");
                    } else {
                        if(a == TOGGLE) {
                            power->setVal(1 - power->getVal());
                            LOG2("      Accessory going %s\n", power->getVal() == 0 ? "ON" : "OFF");
                        } else {
                            power->setVal(1);
                            LOG2("      Accessory going ON\n");
                        }
                    }
                    update();
                    LOG2("      Updating...\n");
                }
                LOG2("   Exit handler\n");
                return;
            } else {
                LOG2("not configured for pin #%d \n", pin);
            }
        }
        LOG2("   Exit handler\n");
    }
};

// =====================================================================================
// DEV_RgbLED: Class for manage dimmable RGB light type devices (PWM based)
// =====================================================================================
struct DEV_RgbLED : Service::LightBulb {  // RGB LED (Common Cathode)

    LedPin *redPin, *greenPin, *bluePin;        // Create references to each color LED anode Pin instantiated below
    SpanCharacteristic *power;                  // reference to the On Characteristic
    SpanCharacteristic *H;                      // reference to the Hue Characteristic
    SpanCharacteristic *S;                      // reference to the Saturation Characteristic
    SpanCharacteristic *V;                      // reference to the Brightness Characteristic
    int actions = 0;                            // pushbutton counter
    BUTTON_ACTION *action[MAX_PUSHBUTTONS];     // rules for pushbuttons
    char *accessoryName;                        // accessory name just for login purpose

    // -----------------------------------------------------------------------------------
    // constructor() method
    //
    // red_pin          reference for the output red anode pin assigned for the class instance
    // green_pin        reference for the output green anode pin assigned for the class instance
    // blue_pin         reference for the output blue anode pin assigned for the class instance
    // -----------------------------------------------------------------------------------
    DEV_RgbLED(int red_pin, int green_pin, int blue_pin, char *name) : Service::LightBulb() {  // constructor() method

        power = new Characteristic::On();
        H = new Characteristic::Hue(0);           // instantiate the Hue Characteristic with an initial value of 0 out of 360
        S = new Characteristic::Saturation(0);    // instantiate the Saturation Characteristic with an initial value of 0%
        V = new Characteristic::Brightness(100);  // instantiate the Brightness Characteristic with an initial value of 100%
        V->setRange(5, 100, 1);                   // sets the range of the Brightness to be from a min of 5%, to a max of 100%, in steps of 1%
        this->accessoryName = name;

        this->redPin = new LedPin(red_pin, 100);      // configures a PWM LED for output to the RED pin
        this->greenPin = new LedPin(green_pin, 100);  // configures a PWM LED for output to the GREEN pin
        this->bluePin = new LedPin(blue_pin, 100);    // configures a PWM LED for output to the BLUE pin

        update();
    }  // end constructor

    // -----------------------------------------------------------------------------------
    // update() method
    //
    // updates hardware pin statuses.
    // -----------------------------------------------------------------------------------
    boolean update() {  // update() method

        boolean p;
        float v, h, s, r, g, b;

        h = H->getVal<float>();  // get and store all current values.  Note the use of the <float> template to properly read the values
        s = S->getVal<float>();
        v = V->getVal<float>();  // though H and S are defined as FLOAT in HAP, V (which is brightness) is defined as INT, but will be re-cast appropriately
        p = power->getVal();

        LOG2("Updating RGB LED: Pins=(%d,%d,%d): ", redPin->getPin(), greenPin->getPin(), bluePin->getPin());

        if (power->updated()) {
            p = power->getNewVal();
            LOG2("Power=%s->%s, ", power->getVal() ? "true" : "false", p ? "true" : "false");
        } else {
            LOG2("Power=%s, ", p ? "true" : "false");
        }

        if (H->updated()) {
            h = H->getNewVal<float>();
            LOG2("H=%.0f->%.0f, ", H->getVal<float>(), h);
        } else {
            LOG2("H=%.0f, ", h);
        }

        if (S->updated()) {
            s = S->getNewVal<float>();
            LOG2("S=%.0f->%.0f, ", S->getVal<float>(), s);
        } else {
            LOG2("S=%.0f, ", s);
        }

        if (V->updated()) {
            v = V->getNewVal<float>();
            LOG2("V=%.0f->%.0f  ", V->getVal<float>(), v);
        } else {
            LOG2("V=%.0f  ", v);
        }

        // Here we call a static function of LedPin that converts HSV to RGB.
        // Parameters must all be floats in range of H[0,360], S[0,1], and V[0,1]
        // R, G, B, returned [0,1] range as well
        LedPin::HSVtoRGB(h, s / 100.0, v / 100.0, &r, &g, &b);  // since HomeKit provides S and V in percent, scale down by 100

        int R, G, B;
        R = p * r * 100;  // since LedPin uses percent, scale back up by 100, and multiple by status fo power (either 0 or 1)
        G = p * g * 100;
        B = p * b * 100;

        LOG2("RGB=(%d,%d,%d)\n", R, G, B);

        redPin->set(100 - R);  // update each ledPin with new values
        greenPin->set(100 - G);
        bluePin->set(100 - B);

        return true;  // return true
    }  // update

    // -----------------------------------------------------------------------------------
    // setActionsOnSpanButton() method
    //
    // Attachs action configuration to a specific pin
    //
    // controlPin       reference for the input button pin assigned for the class instance
    // actionOnLong     action to be done on long press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnSingle   action to be done on normal press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // actionOnDouble   action to be done on quick double click press on the asociated button (ON, OFF, TOGGLE or NOTHING)
    // -----------------------------------------------------------------------------------
    boolean setActionsOnSpanButton(int controlPin, int actionOnLong, int actionOnSingle, int actionOnDouble) {

        if (actions == MAX_PUSHBUTTONS) {
            return false;                   // Only MAX_PUSHBUTTONS actions supported per accesory
        }
        WEBLOG("Configuring ON/OFF for RGB Led accessory %s[Pin#(%d,%d,%d)] with control pushbutton on pin %d and index #%d", accessoryName, redPin->getPin(), greenPin->getPin(), bluePin->getPin(), controlPin, actions);
        WEBLOG("      %s on normal click", Action2Name(actionOnSingle));
        WEBLOG("      %s on double click", Action2Name(actionOnDouble));
        WEBLOG("      %s on long press", Action2Name(actionOnLong));
        new SpanButton(controlPin, LONGPULSE, MINIMUMPULSE, SHORTPULSE);
        this->action[actions] = new BUTTON_ACTION(controlPin, actionOnLong, actionOnSingle, actionOnDouble);
        actions++;
        return true;
    }

    // -----------------------------------------------------------------------------------
    // button() method
    //
    // handles clic on button. Library pass button pin id and press type so its responsability
    // of the handler to decide what to do with the button an press type
    // -----------------------------------------------------------------------------------
    void button(int pin, int pressType) override {

        LOG2("Accessory %s on pins (%d, %d, %d) button handler got button press on pin %d with type %s\n", accessoryName, redPin->getPin(), greenPin->getPin(), bluePin->getPin(), pin, pressType == SpanButton::LONG ? "LONG" : (pressType == SpanButton::SINGLE ? "SINGLE" : (pressType == SpanButton::DOUBLE ? "DOUBLE" : "???")));
        for (int i = 0; i < this->actions; i++) {
            LOG2("   Pushbutton index #%d ", i);
            if (pin == action[i]->controlPin) {
                LOG2("configured for pin #%d ", pin);
                int a = ((pressType == SpanButton::SINGLE) ? action[i]->actionOnSingle : ((pressType == SpanButton::DOUBLE) ? action[i]->actionOnDouble : ((pressType == SpanButton::LONG) ? action[i]->actionOnLong : NOTHING)));
                LOG2("with action: #%s\n", Action2Name(a));
                if(a == NOTHING) {
                    LOG2("      Nothing to do\n");
                } else {
                    if(a == OFF) {
                        power->setVal(0);
                        H = new Characteristic::Hue(0);
                        S = new Characteristic::Saturation(0);
                        V = new Characteristic::Brightness(0);
                        LOG2("      Accessory going OFF\n");
                        update();
                        LOG2("      Updating...\n");
                    } else {
                        if(a == TOGGLE) {
                            LOG2("Not supported\n");
                        } else {
                            LOG2("Not supported\n");
                        }
                    }
                }
                LOG2("   Exit handler\n");
                return;
            } else {
                LOG2("not configured for pin #%d \n", pin);
            }
        }
        LOG2("   Exit handler\n");
    }
};
