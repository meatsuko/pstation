#include "Arduino.h"

void initVariant() __attribute__((weak));
void initVariant() { }

const int PIN_TEMP_COLLIBRATION = A3;
const int PIN_TEMP_HEATER = A5;

const int PIN_PWM_HEATER = 11;

unsigned int CF_TEMP_SETTED = 200;
unsigned int CF_PWM_AMPLITUDE = 255;

int getTemperatureHeater();
void customPrintLn(String type, int message);

unsigned char encoder_CLK;
unsigned char encoder_DT;
unsigned char encoder_CLK_ = 0;

// Interrupts 6 priority
ISR(PCINT2_vect)
{
    encoder_CLK = digitalRead(4);
    encoder_DT = digitalRead(5);
    if((!encoder_CLK) && (encoder_CLK_))
    {
        if(encoder_DT){ if(CF_TEMP_SETTED >= 50) CF_TEMP_SETTED -= 5; }
        else { if(CF_TEMP_SETTED <= 390) CF_TEMP_SETTED += 5; }
        customPrintLn("TEMP_SETTED", CF_TEMP_SETTED);
    }
    encoder_CLK_ = encoder_CLK;
}

int main(void)
{
    init();
    initVariant();

    *digitalPinToPCMSK(4) |= bit(digitalPinToPCMSKbit(4));
    *digitalPinToPCMSK(5) |= bit(digitalPinToPCICRbit(5));
    PCIFR |= bit(digitalPinToPCICRbit(4)) | bit(digitalPinToPCICRbit(5));
    PCICR |= bit(digitalPinToPCICRbit(4)) | bit(digitalPinToPCICRbit(5));

    Serial.begin(115200);
    do
    {
        if(getTemperatureHeater() < CF_TEMP_SETTED)
            if((CF_TEMP_SETTED - getTemperatureHeater()) < 16 && (CF_TEMP_SETTED - getTemperatureHeater()) > 6) CF_PWM_AMPLITUDE = 100;
            else if((CF_TEMP_SETTED - getTemperatureHeater()) < 8 && (CF_TEMP_SETTED - getTemperatureHeater()) > 4) CF_PWM_AMPLITUDE = 80;
            else if((CF_TEMP_SETTED - getTemperatureHeater()) < 4) CF_PWM_AMPLITUDE = 45;
            else CF_PWM_AMPLITUDE = 230;
        else CF_PWM_AMPLITUDE = 0;

        analogWrite(PIN_PWM_HEATER, CF_PWM_AMPLITUDE);

        customPrintLn("TEMP", getTemperatureHeater());
        customPrintLn("TEMP_SETTED", CF_TEMP_SETTED);
        customPrintLn("PWM", CF_PWM_AMPLITUDE);

        delay(500);

        if(serialEventRun) serialEventRun();
    }
    while (true);

    return 0;
}

int getTemperatureHeater()
{
    return map(((analogRead(PIN_TEMP_HEATER) + analogRead(PIN_TEMP_COLLIBRATION)) - 512), 0, 430, 25, 310);
    //надо бы сюда еще constrain() накинуть
}

void customPrintLn(String type, int message)
{
    Serial.print("$");
    Serial.print(type);
    Serial.print(": ");
    Serial.print(message);
    Serial.println(";");
}
