#include <Wire.h>
#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS 150
#define DATA_PIN 6

// Mode enumeration - if you want to add additional party or colour modes, add them here; you'll need to map some IR codes to them later; and add the modes into the main switch loop
enum Mode {CLOUD,ACID,OFF,ON,TEMPEST,UPBRIGHT,DOWNBRIGHT,WHITE,FADE,RED,GREEN,BLUE,RED1,GREEN1,BLUE1};
Mode mode = WHITE;
Mode lastMode = WHITE;

// Mic settings, shouldn't need to adjust these.
#define MIC_PIN   0   // Microphone is attached to this analog pin
#define DC_OFFSET  0  // DC offset in mic signal - if unusure, leave 0
#define NOISE     10  // Noise/hum/interference in mic signal
#define SAMPLES   10  // Length of buffer for dynamic level adjustment
byte
volCount  = 0;      // Frame counter for storing past volume data
int
vol[SAMPLES];       // Collection of prior volume samples
int      n, total = 30;
float average = 0;

// used to make basic mood lamp colour fading feature
int fade_h;
int fade_direction = 1;

int pushed = 0;
int current = 0;


// Define the array of leds
CRGB leds[NUM_LEDS];






// Parameters for a real thunder simulation
boolean isStarting = false;
boolean isGrowing = false;
boolean isTempest = false;
boolean isCalm = false;
boolean isReducing = false;
boolean isEnding = false;
boolean isWaiting = false;
int _1mn = 60000;
int _2mn = _1mn*2;
int _3mn = _1mn*3;
int _4mn = _1mn*4;
int _5mn = _1mn*5;
int _6mn = _1mn*6;
int _7mn = _1mn*7;
int _8mn = _1mn*8;
int _9mn = _1mn*9;
int _10mn = _1mn*10;
int _11mn = _1mn*11;
int _12mn = _1mn*12;
int _13mn = _1mn*13;
int _14mn = _1mn*14;
int _15mn = _1mn*15;
unsigned long previousMillis=0;

int loopTime = 21600000; //(6H) || 86400000 (24H)
int currentTime = 0;
int _intensity = 50;

int _thunderDelayMin = 10;
int _thunderDelayMax = 500;
int _crackDelayMin = 50;
int _crackDelayMax = 250;



void setup() {
    delay(2000);
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    Serial.begin(115200);
    Wire.begin(9); // Start I2C Bus as a Slave (Device Number 9)
    Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes) {
    // Here, we set the mode based on the IR signal received. Check the debug log when you press a button on your remote, and  add the hex code here (you need 0x prior to each command to indicate it's a hex value)
    while(Wire.available()){
        unsigned int received = Wire.read();
        Serial.print("Receiving IR hex: ");
        Serial.println(received,HEX);
        lastMode = mode;
        switch(received){
            case 0x3F:
                mode = ON; break;
            case 0xBF:
                mode = OFF; break;
            case 0x2F:
                mode = CLOUD; break;
            case 0xF:
                mode = ACID; break;
            case 0x37:
                mode = FADE; break;
            case 0x4F // might be put on last remote button
                currentTime = millis();;
                mode = TEMPEST; break;
            case 0xDF:
                mode = RED; break;
            case 0x5F:
                mode = GREEN; break;
            case 0x9F:
                mode = BLUE; break;
            case 0x1F:
                mode = WHITE; break;
            case 0xEF:
                mode = RED1; break;
            case 0x6F:
                mode = GREEN1; break;
            case 0xAF:
                mode = BLUE1; break;
            case 0xFF:
                pushed = 1;
                mode = UPBRIGHT;
                break;
            case 0x7F:
                pushed = 1;
                mode = DOWNBRIGHT;
                break;
        }
    }
}

void loop() {
    // Maps mode names to code functions.
    switch(mode){
        case CLOUD: detect_thunder();reset();break;
        case ACID: acid_cloud();reset();break;
        case OFF:reset();break;
        case ON: thunder();reset();break;
        case WHITE: _white();break;
        case RED: single_colour(0);break;
        case GREEN: single_colour(96);break;
        case BLUE: single_colour(160);break;
        case RED1: dim_colour(20);break;
        case GREEN1: dim_colour(116);break;
        case BLUE1: dim_colour(180);break;
        case FADE: colour_fade();break;
        case UPBRIGHT: _upbright();break;
        case DOWNBRIGHT: _downbright();break;
        default: _white();break;
    }
}

void dim_colour(int H){
    for (int i=0;i<NUM_LEDS;i++){
        //leds[i] = CHSV( H, 255, 255);
        leds[i].fadeToBlackBy(8);
        //leds[i].maximizeBrightness();
    }
    //avoid flickr which occurs when FastLED.show() is called - only call if the colour changes

    FastLED.show();
    delay(150);
}

void _white(){
    if(lastMode != mode){
        lastMode = mode;
    }
    if(current<=NUM_LEDS){
        for(int i = 0; i < NUM_LEDS; i++) {
            // Turn our current led on to white, then show the leds
            //leds[i] = CRGB::White;
            leds[i] = CHSV( 200, 100, 120);
            FastLED.show();
            delay(10);
            // Turn our current led back to black for the next loop around
            //leds[i] = CRGB::Black;
            current = i;
        }
    } else {
        for (int i=0;i<NUM_LEDS;i++){
            leds[i] = CHSV( 200, 100, 120);
            //leds[i] = CHSV( 128, 128, 255);
        }
        FastLED.show();
        delay(50);
    }
}

void _upbright(){
    if(pushed==1){
        for(int i=0;i<NUM_LEDS;i++) {
            leds[i].fadeLightBy(64);
        }
        FastLED.show();
    }
    pushed = 0;
}

void _downbright(){
    if(pushed==1){
        for(int i=0;i<NUM_LEDS;i++) {
            leds[i].fadeToBlackBy(64);
        }
        FastLED.show();
    }
    pushed = 0;
}

void single_colour(int H){
    for (int i=0;i<NUM_LEDS;i++){
        leds[i] = CHSV( H, 255, 255);
    }
    //avoid flickr which occurs when FastLED.show() is called - only call if the colour changes
    if(lastMode != mode){
        FastLED.show();
        lastMode = mode;
    }
    delay(50);
}

void colour_fade(){
    //mood mood lamp that cycles through colours
    for (int i=0;i<NUM_LEDS;i++){
        leds[i] = CHSV(fade_h, 255, 255);
    }
    if(fade_h >254){
        fade_direction = -1; //reverse once we get to 254
    }else if(fade_h < 0){
        fade_direction = 1;
    }
    fade_h += fade_direction;
    Serial.println("FADE VALUE");
    Serial.println(fade_h);
    FastLED.show();
    delay(100);
}



void detect_thunder() {

    n   = analogRead(MIC_PIN);              // Raw reading from mic
    n   = abs(n - 512 - DC_OFFSET);         // Center on zero
    n   = (n <= NOISE) ? 0 : (n - NOISE);   // Remove noise/hum
    vol[volCount] = n;                      // Save sample for dynamic leveling
    if(++volCount >= SAMPLES) volCount = 0; // Advance/rollover sample counter

    total = 0;
    for(int i=0; i<SAMPLES; i++) {
        total += vol[i];
    }

    average = (total/SAMPLES)+2;
    if(n>average){
        Serial.println("TRIGGERED");
        reset();
        //I've programmed 3 types of lightning. Each cycle, we pick a random one.
        switch(random(1,3)){
            case 1:
                thunderburst();
                delay(random(10,500));
                Serial.println("Thunderburst");
                break;
            case 2:
                rolling();
                Serial.println("Rolling");
                break;
            case 3:
                crack();
                delay(random(50,250));
                Serial.println("Crack");
                break;
        }
    }
}


// utility function to turn all the lights off.
void reset(){
    for (int i=0;i<NUM_LEDS;i++){
        leds[i] = CHSV( 0, 0, 0);
    }
    current = 0;
    pushed = 0;
    _thunderDelayMin = 10;
    _thunderDelayMax = 500;
    _crackDelayMin = 50;
    _crackDelayMax = 250;
    FastLED.show();
}

void acid_cloud(){
    for(int i=0;i<NUM_LEDS;i++){
        if(random(0,100)>90){
            leds[i] = CHSV( random(0,255), 255, 255);
        }else{
            leds[i] = CHSV(0,0,0);
        }
    }
    FastLED.show();
    delay(random(5,100));
    reset();
}

void rolling(){
    // a simple method where we go through every LED with 1/10 chance of being turned on, up to 10 times, with a random delay between each time
    for(int r=0;r<random(2,10);r++){
        for(int i=0;i<NUM_LEDS;i++){
            if(random(0,100)>90){
                leds[i] = CHSV( 0, 0, 255);
            }else{
                //dont need reset as we're blacking out other LEDs her
                leds[i] = CHSV(0,0,0);
            }
        }
        FastLED.show();
        delay(random(5,100));
        reset();
    }
}

void crack(){
    //turn everything white briefly
    for(int i=0;i<NUM_LEDS;i++) {
        leds[i] = CHSV( 0, 0, 255);
    }
    FastLED.show();
    delay(random(10,100));
    reset();
}

void thunderburst(int intensity){
    // this thunder works by lighting two random lengths of the strand from 10-20 pixels.
    int rs1 = random(0,NUM_LEDS/2);
    int rl1 = random(10,20);
    int rs2 = random(rs1+rl1,NUM_LEDS);
    int rl2 = random(10,20);

    //repeat this chosen strands a few times, adds a bit of realism
    for(int r = 0;r<random(3,6);r++){

        for(int i=0;i< rl1; i++){
            leds[i+rs1] = CHSV( 0, 0, intensity);
        }

        if(rs2+rl2 < NUM_LEDS){
            for(int i=0;i< rl2; i++){
                leds[i+rs2] = CHSV( 0, 0, intensity);
            }
        }

        FastLED.show();
        //stay illuminated for a set time
        delay(random(10,50));

        reset();
        delay(random(10,50));
    }

}

void thunder(int intensity, int thunderDelayMin, int thunderDelayMax, int crackDelayMin, int crackDelayMax){


    switch(random(1,3)){
        case 1:
            thunderburst(intensity);
            delay(random(10,500));
            Serial.println("Thunderburst");
            break;
        case 2:
            rolling();
            Serial.println("Rolling");
            break;
        case 3:
            crack();
            delay(random(50,250));
            Serial.println("Crack");
            break;
    }
}

void tempest_simulation(){
    setParamsTempest();
    thunder(_intensity, _thunderDelayMin, _thunderDelayMax, _crackDelayMin, _crackDelayMax);
}

void setParamsTempest(){
    isStarting = false;
    isGrowing = false;
    isTempest = false;
    isCalm = false;
    isReducing = false;
    isEnding = false;

    if(currentTime >= 0 && currentTime <=_2mn){
        intensityTempest = random(20,100);
        isStarting = true;
    } else if(currentTime > _2mn && currentTime <=_5mn){
        intensityTempest = random(60,180);
        isGrowing = true;
    } else if(currentTime > _5mn && currentTime <=_7mn){
        intensityTempest = random(190,255);
        isTempest = true;
    } else if(currentTime > _7mn && currentTime <=_8mn){
        intensityTempest = random(140,210);
        isCalm = true;
    } else if(currentTime > _8mn && currentTime <=_10mn){
        intensityTempest = random(180,255);
        isTempest = true;
    } else if(currentTime > _10mn && currentTime <=_13mn){
        intensityTempest = random(90,160);
        isReducing = true;
    } else if(currentTime > _13mn && currentTime <=_15mn){
        intensityTempest = random(20,80);
        isEnding = true;
    }
    
    if(millis() < previousMillis){ // Reinit if millis() reach the 50 days
        previousMillis = 0;
    }
    
    if(currentTime >= loopTime){
        currentTime = 0;
        previousMillis = previousMillis + loopTime;
    } else {
        currentTime = millis() - previousMillis;  
    }

}
