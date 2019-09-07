#include "JeeUI2.h"


void jeeui2::formMqtt(){
    text("m_host", "MQTT host");
    number("m_port", "MQTT port");
    text("m_user", "User");
    text("m_pass", "Password");
    button("bWF", "gray", "Reconnect");
}

void jeeui2::formWifi(){
    text("ssid", "SSID");
    password("pass", "Password");
    button("bWF", "gray", "Connect");
}



void jeeui2::app(String name){
    mn = 0;
    pg = 0;
    buf = "{\"app\":\"" + name + "\",";
    buf += "\"ID\":\"" + mc + "\",";
}

void jeeui2::text(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"text\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::number(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"number\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::time(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"time\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::date(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"date\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::datetime(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"datetime-local\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::range(String id, int min, int max, float step, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"min\":\"" + String(min) + "\",";
    buf += "\"max\":\"" + String(max) + "\",";
    buf += "\"step\":\"" + String(step) + "\",";
    buf += "\"type\":\"range\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::email(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"email\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::password(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"password\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::option(String value, String label){
    op += "{\"label\":\"" + label + "\",\"value\":\"" + value + "\"},";
}

void jeeui2::select(String id, String label){

    int lastIndex = op.length() - 1;
    op.remove(lastIndex);

    buf += "{\"html\":\"select\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\",";
    buf += "\"options\":[" + op + "]";
    buf += "},";
    op = "";
}

void jeeui2::checkbox(String id, String label){
    buf += "{\"html\":\"input\",";
    buf += "\"type\":\"checkbox\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}
void jeeui2::color(String id, String label){

    buf += "{\"html\":\"input\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"color\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}
void jeeui2::button(String id, String color, String label){
    buf += "{\"html\":\"button\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"color\":\"" + color + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::button(String id, String color, String label, int column){
    buf += "{\"html\":\"button\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"color\":\"" + color + "\",";
    buf += "\"label\":\"" + label + "\",";
    buf += "\"col\":\"" + String(column) + "\"";
    buf += "},";
}

void jeeui2::textarea(String id, String label){
    buf += "{\"html\":\"textarea\",";
    buf += "\"id\":\"" + id + "\",";
    buf += "\"type\":\"text\",";
    buf += "\"value\":\"" + param(id) + "\",";
    buf += "\"label\":\"" + label + "\"";
    buf += "},";
}

void jeeui2::menu(String name){
    
    if (mn == 0)
        buf += "\"menu\":[";
    else{
        int lastIndex = buf.length() - 1;
        buf.remove(lastIndex);
        buf.remove(lastIndex - 1);
        buf += ",";
    }
    buf += "\"" + name + "\"],";
    mn++;
}

void jeeui2::page(){
    if (pg == 0)
        buf += "\"content\":[[";
    else{
        int lastIndex = buf.length() - 1;
        buf.remove(lastIndex);
        buf += "],[";
    }

    if (pg == mn){
        int lastIndex = buf.length() - 1;
        buf.remove(lastIndex);
        buf.remove(lastIndex - 1);
        buf.remove(lastIndex - 2);
        //buf.remove(lastIndex - 3);
        buf += "]]}";
    }
    else
        pg++;
}