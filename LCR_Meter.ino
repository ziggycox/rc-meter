/*
 Class Project code for LCR meter
 Version: 7/17/18
 Author: Terry Cox
*/

#include <LiquidCrystalFast.h>
#include <ADC.h>

         // LCD pins: RS  RW  EN  D4 D5 D6 D7
LiquidCrystalFast lcd(12, 10, 11, 5, 4, 3, 2);

ADC *adc = new ADC();

int mode, val = 0;
unsigned long start1, stop1, elapsed;
int readPin = A9;                        // Analog_In
int pinOut = 22;                         //DigitalWrite out pin

// set up mux
int r0,r1,r2, resistorPin = 0;
int s0 = 19, s1 = 18, s2 = 17;     //mapping of select pins to mux

// Internal mux Resistance
//int muxRes = 420;    //254 ohm is what I measured. This value seems to work better  (420) for under 100k resistor

// Define resistor values
#define res100 1
#define res1k 0
#define res10k 2
#define res100k 3
#define res1m 4
#define res10m 5
#define res100m 6
#define res1g 7

void setup() {
    lcd.begin(16, 2);
    pinMode(pinOut, OUTPUT);
    pinMode(readPin, INPUT);
    
    // Set up buttons
    pinMode(6, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);

    //mux
    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);

    //ADC0
    adc->setAveraging(16);
    adc->setResolution(12);
    adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED);    //MED
    adc->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED);   //Very high
    //adc->enableCompare(1.0/3.3*adc->getMaxValue(ADC_0), 0, ADC_0);
    //adc->enableCompareRange(1.0*adc->getMaxValue(ADC_0)/3.3, 2.0*adc->getMaxValue(ADC_0)/3.3, 0, 1, ADC_0);

    Serial.begin(9600);
    delay(500);
}

void loop() {
    if((digitalRead(6) == LOW) && (mode != 1)) {
        lcd.clear();
        lcd.print("Resistor");
        mode = 1;
        readResistor();
    }
    if((digitalRead(7) == LOW) && (mode != 2)) {
        lcd.clear();
        lcd.print("Capacitor");
        mode = 2;
        readCap();
    }
    if((digitalRead(8) == LOW) && (mode != 3)) {
        lcd.clear();
        lcd.print("Inductor");
        mode = 3;
    }
}


void readCap() {
    while((digitalRead(6) == HIGH) && (digitalRead(8) == HIGH)) {
        val = 0;

        //mux select
        r0 = bitRead(res10k,0);
        r1 = bitRead(res10k,1);
        r2 = bitRead(res10k,2);
        digitalWrite(s0, r0);
        digitalWrite(s1, r1);
        digitalWrite(s2, r2);
    
        //lcd.print("Farad");
        //lcd.setCursor(0,1);
        digitalWrite(pinOut, HIGH);
        start1 = micros();
        while(val <= 2588) {
            val = adc->adc0->analogRead(readPin);
        }
        stop1 = micros();
        digitalWrite(pinOut, LOW);
        elapsed = stop1 - start1;
        //lcd.print(elapsed);
        double temp = elapsed / 11050.0;
        lcd.print(temp);
        lcd.setCursor(0,1);
        lcd.print("uF");
        delay(2000);
        lcd.clear();
    }
}

void readResistor() {
    digitalWrite(pinOut, HIGH);

    
    while((digitalRead(7) == HIGH) && (digitalRead(8) == HIGH)) {
        int resisValue = resistorSelect();
        //val = adc->adc0->analogRead(readPin);
        //double voltage = val*(3.3/adc->getMaxValue(ADC_0));
        double voltageAvg[100];
        for(int i = 0; i < 100; ++i) {
            val = adc->adc0->analogRead(readPin);
            voltageAvg[i] = val*(3.3/adc->getMaxValue(ADC_0));
        }
        double temp = 0;
        for(int i = 0; i < 100; ++i) {
            temp += voltageAvg[i];
        }
        double voltage = temp / 100.0;
        displayR(voltage, resisValue);
        delay(500);
        //lcd.clear();
    }
}

void displayR(double voltage, int resisValue) {
    double resistance = ((resisValue) * (1 / ((3.3 / voltage) - 1)));
    //lcd.print(resisValue);
    //delay(5000);
    
    
    // Autoranging
    int tempDis = resistance/1000;
    lcd.clear();
    if((tempDis >= 1) && (tempDis < 1000)) {
        resistance /= 1000;
        lcd.print(resistance);
        lcd.setCursor(0,1);
        lcd.print("K Ohm");
    } else if(tempDis >= 1000) {
        resistance /= 1000000;
        lcd.print(resistance);
        lcd.setCursor(0,1);
        lcd.print("M Ohm");
    } else {
        lcd.print(resistance);
        lcd.setCursor(0,1);
        lcd.print("Ohm");
    }
}

int resistorSelect() {
    // start with 10K resistor
    r0 = bitRead(res10k,0);
    r1 = bitRead(res10k,1);
    r2 = bitRead(res10k,2);
    digitalWrite(s0, r0);
    digitalWrite(s1, r1);
    digitalWrite(s2, r2);

    // Read in voltage + calculate resistance
    val = adc->adc0->analogRead(readPin);
    double voltage = val*(3.3/adc->getMaxValue(ADC_0));
    double resistance = ((10000+420) * (1 / ((3.3 / voltage) - 1)));

    if((resistance < 7000) || ((95000 < resistance)&&(resistance < 105000))) {
        return 10420;
    } else if ((resistance > 7000) && (resistance < 30000)) {
        r0 = bitRead(res100k,0);
        r1 = bitRead(res100k,1);
        r2 = bitRead(res100k,2);
        digitalWrite(s0, r0);
        digitalWrite(s1, r1);
        digitalWrite(s2, r2);

        return 100420;
    } else {
        r0 = bitRead(res1k,0);
        r1 = bitRead(res1k,1);
        r2 = bitRead(res1k,2);
        digitalWrite(s0, r0);
        digitalWrite(s1, r1);
        digitalWrite(s2, r2);

        return 1950;
    } 
    
}

