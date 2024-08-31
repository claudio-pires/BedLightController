#include "HomeSpan.h"        
#include "DEV_LED.h"          

#define SWITCH_PIN_0 16                     // Pin for main room pushbutton
#define SWITCH_PIN_1 17                     // Pin for side table #1 pushbutton 
#define SWITCH_PIN_2 27                     // Pin for side table #2 pushbutton 
#define CEILING_LAMP_PIN 32                 // Output for ceiling lamp relay
#define READING_LAMP1_PIN 33                // Output for reading lamp #1 relay
#define READING_LAMP2_PIN 25                // Output for reading lamp #2 relay
#define STANDING_LAMP_PIN 26                // Output for standing lamp relay
#define HEADBOARD_LAMP_RED_ANODE_PIN 21     // PWM output for red control of the led stripe
#define HEADBOARD_LAMP_GREEN_ANODE_PIN 22   // PWM output for green control of the led stripe
#define HEADBOARD_LAMP_BLUE_ANODE_PIN 23    // PWM output for blue control of the led stripe
#define STATUS_LED_PIN 18                   // Status led pin
#define CONTROL_BUTTON_PIN 19               // Control button pin

#define DEFAULT_PAIRING_CODE "35674583"      // Pairing code

#define WEBLOG_BUFFER_SIZE 50
#define DEFAULT_LOG_LEVEL 2

// Accesories 
DEV_LED *ceilingLamp, *readingLamp1, *readingLamp2, *standingLamp;
DEV_RgbLED *ledStripe;


void setup() {                
 
    Serial.begin(115200);       

    homeSpan.setPairingCode(DEFAULT_PAIRING_CODE);
    homeSpan.setControlPin(CONTROL_BUTTON_PIN);
    homeSpan.setStatusPin(STATUS_LED_PIN);
    homeSpan.setLogLevel(DEFAULT_LOG_LEVEL);
    homeSpan.enableWebLog(WEBLOG_BUFFER_SIZE,"pool.ntp.org","UTC","log");

    homeSpan.begin(Category::Bridges,"Bedroom lighting controller"); 

    SPAN_ACCESSORY()                                              // More than 3 accesories requires add bridge accesory an delect Bridges as category

    SPAN_ACCESSORY("Ceiling lamp")
    ceilingLamp = new DEV_LED(CEILING_LAMP_PIN, "Ceiling Lamp");                // Creates accessory with activity on pin CEILING_LAMP_PIN
    ceilingLamp->setActionsOnSpanButton(SWITCH_PIN_0, OFF, TOGGLE, OFF);        // Long press on SWITCH_PIN_0 makes accessory off, normal press makes 
                                                                                // accesory toggle status, double click makes accesory goes off
    ceilingLamp->setActionsOnSpanButton(SWITCH_PIN_1, OFF, NOTHING, NOTHING);   // Long press on SWITCH_PIN_1 makes accessory off, normal press does not
                                                                                // have effect on accesory, double click makes accesory going off
    ceilingLamp->setActionsOnSpanButton(SWITCH_PIN_2, OFF, NOTHING, NOTHING);   // Long press on SWITCH_PIN_2 makes accessory off, normal press does not
                                                                                // have effect on accesory, double click makes accesory going off

    SPAN_ACCESSORY("#1 Reading lamp")
    readingLamp1 = new DEV_LED(READING_LAMP1_PIN, "#1 Reading lamp");           // Creates accessory with activity on pin READING_LAMP1_PIN
    readingLamp1->setActionsOnSpanButton(SWITCH_PIN_0, OFF, NOTHING, TOGGLE);   // Long press on SWITCH_PIN_0 makes accessory off, normal press does 
                                                                                // nothing, double click makes accesory going on
    readingLamp1->setActionsOnSpanButton(SWITCH_PIN_1, OFF, TOGGLE, NOTHING);   // Long press on SWITCH_PIN_1 makes accessory off, normal press makes 
                                                                                // accesory toggle status, double click makes accesory going off
    readingLamp1->setActionsOnSpanButton(SWITCH_PIN_2, OFF, NOTHING, NOTHING);  // Long press on SWITCH_PIN_2 makes accessory off, normal press does 
                                                                                // nothing on accesory, double click makes accesory going off

    SPAN_ACCESSORY("#2 Reading lamp")
    readingLamp2 = new DEV_LED(READING_LAMP2_PIN, "#2 Reading lamp");           // Creates accessory with activity on pin READING_LAMP2_PIN
    readingLamp2->setActionsOnSpanButton(SWITCH_PIN_0, OFF, NOTHING, TOGGLE);   // Long press on SWITCH_PIN_0 makes accessory off, normal press does 
                                                                                // nothing, double click makes accesory going on
    readingLamp2->setActionsOnSpanButton(SWITCH_PIN_1, OFF, NOTHING, NOTHING);  // Long press on SWITCH_PIN_1 makes accessory off, normal press does 
                                                                                // nothing on accesory, double click makes accesory going off
    readingLamp2->setActionsOnSpanButton(SWITCH_PIN_2, OFF, TOGGLE, NOTHING);   // Long press on SWITCH_PIN_2 makes accessory off, normal press makes 
                                                                                // accesory toggle status, double click makes accesory going off

    SPAN_ACCESSORY("Standing lamp")
    standingLamp = new DEV_LED(STANDING_LAMP_PIN, "Standing lamp");             // Creates accessory with activity on pin STANDING_LAMP_PIN
    standingLamp->setActionsOnSpanButton(SWITCH_PIN_0, OFF, NOTHING, TOGGLE);   // Long press on SWITCH_PIN_0 makes accessory off, normal press does 
                                                                                // nothing, double click makes accesory going on
    standingLamp->setActionsOnSpanButton(SWITCH_PIN_1, OFF, NOTHING, TOGGLE);   // Long press on SWITCH_PIN_1 makes accessory off, normal press makes 
                                                                                // accesory toggle status, double click makes accesory toggle status
    standingLamp->setActionsOnSpanButton(SWITCH_PIN_2, OFF, NOTHING, TOGGLE);   // Long press on SWITCH_PIN_2 makes accessory off, normal press makes 
                                                                                // accesory toggle status, double click makes accesory toggle status

    SPAN_ACCESSORY("Headboard lamp")
    ledStripe = new DEV_RgbLED(HEADBOARD_LAMP_RED_ANODE_PIN, HEADBOARD_LAMP_GREEN_ANODE_PIN, 
                               HEADBOARD_LAMP_BLUE_ANODE_PIN, "Headboard lamp");  // Creates accessory with activity on pin STANDING_LAMP_PIN
    ledStripe->setActionsOnSpanButton(SWITCH_PIN_0, OFF, NOTHING, NOTHING);       // Long press on SWITCH_PIN_0 makes accessory off, normal and double clic 
                                                                                  // do nothing
    ledStripe->setActionsOnSpanButton(SWITCH_PIN_1, OFF, NOTHING, NOTHING);       // Long press on SWITCH_PIN_1 makes accessory off, normal and double clic 
                                                                                  // do nothing
    ledStripe->setActionsOnSpanButton(SWITCH_PIN_2, OFF, NOTHING, NOTHING);       // Long press on SWITCH_PIN_2 makes accessory off, normal and double clic 
                                                                                  // do nothing
}

void loop() {
  homeSpan.poll();         // run HomeSpan!
} 

 