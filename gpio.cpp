#include "JeeUI2.h"



void jeeui2::led_handle()
{
    if (LED_PIN == -1) return;
    if (wifi_mode == 1)
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    if (wifi_mode == 2)
        digitalWrite(LED_PIN, LOW + LED_INVERT);
}


void jeeui2::btnCallback(String name, buttonCallback response)
{
    if (name == "GPIO0" && !digitalRead(BUTTON)){
        response();
        btn();
    }

    if (btnui == name){
        btnui = "";
        response();
    }
}

void jeeui2::btn()
{
    if (digitalRead(BUTTON))
        return;

    uint8_t old_wifi_mode = wifi_mode;

    led_inv();
    unsigned long t = millis();
    bool i = false;
    while (!digitalRead(BUTTON))
    {
        delay(1);
        if (t + 5000 < millis()) // Нажатие 5 секунд
        {
            
            if(!i){
                led_inv();
                i = true;
            }
            if (old_wifi_mode == 1)
                wifi_mode = 2;
            if (old_wifi_mode == 2)
                wifi_mode = 1;
        }
        if (t + 15000 < millis()) // Нажатие 10 секунд
        {
            led_inv();
            LittleFS.remove("/config.json");
            ESP.restart();
        }
    }
    if (wifi_mode != old_wifi_mode) // событие после отпуска кнопки от 5 секунд
    {
        if (wifi_mode == 1)
            var("wifi", "AP");
        if (wifi_mode == 2)
            var("wifi", "STA");
            // тут сохранить настройки
        wifi_connect();
        //ESP.restart();
    }
}

void jeeui2::led_on(){
    if (LED_PIN == -1) return;
    digitalWrite(LED_PIN, false);
}

void jeeui2::led_off(){
    if (LED_PIN == -1) return;
    digitalWrite(LED_PIN, true);
}

void jeeui2::led_inv(){
    if (LED_PIN == -1) return;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void jeeui2::ui(void (*uiFunction) ())
{
    foo = uiFunction;
}

void testFunction(){

}
