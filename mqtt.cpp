#include "JeeUI2.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <PubSubClient.h>

bool _t_inc_current = false;
String _t_tpc_current;
String _t_pld_current;

typedef void (*mqttCallback) (String topic, String payload);
mqttCallback mqt;

typedef void (*rcCallback) (String topic, String payload);
rcCallback rctrl;

typedef void (*emptyFunc) (String topic, String payload);
emptyFunc eFunc;

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_callback(char* topic, byte* payload, unsigned int length) ;

String jeeui2::id(String topic){
    if(topic == "") return topic;
    else return m_pref + '/' + topic;
}

void emptyFunction(String, String){}

void (jeeui2::*func)(String topic, String payload);

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), bool remotecontrol){
    if(host != "" && port != 0) mqtt_enable = true;
    String _m_pref = param("m_pref");
    String _m_host = param("m_host");
    String _m_port = param("m_port");
    String _m_user = param("m_user");
    String _m_pass = param("m_pass");
    if(_m_pref == "null") var("m_pref", pref);
    if(_m_host == "null") var("m_host", host);
    if(_m_port == "null") var("m_port", String(port));
    if(_m_user == "null") var("m_user", user);
    if(_m_pass == "null") var("m_pass", pass);

    if (remotecontrol) remoteControl = true;
    mqt = mqttFunction;

}

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload)){
    mqtt(pref, host, port, user, pass, mqttFunction, false);
}

void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload)){
    getAPmac();
    mqtt(mc, host, port, user, pass, mqttFunction, false);
}

void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), bool remotecontrol){
    getAPmac();
    mqtt(mc, host, port, user, pass, mqttFunction, remotecontrol);
}

void jeeui2::mqtt(String host, int port, String user, String pass, bool remotecontrol){
    getAPmac();
    mqtt(mc, host, port, user, pass, emptyFunction, remotecontrol);
}

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, bool remotecontrol){
    mqtt(pref, host, port, user, pass, emptyFunction, remotecontrol);
}

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) (), bool remotecontrol){
    onConnect = mqttConnect;
    mqtt(pref, host, port, user, pass, emptyFunction, remotecontrol);

}
void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) ()){
    onConnect = mqttConnect;
    mqtt(pref, host, port, user, pass, mqttFunction, false);
}
void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) ()){
    onConnect = mqttConnect;
    mqtt(mc, host, port, user, pass, mqttFunction, false);
}
void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) (), bool remotecontrol){
    onConnect = mqttConnect;
    mqtt(mc, host, port, user, pass, mqttFunction, remotecontrol);
}

void jeeui2::mqtt_update(){
    if(!mqtt_enable) return;
    m_pref = param("m_pref");
    m_host = param("m_host");
    m_port = param("m_port").toInt();
    m_user = param("m_user");
    m_pass = param("m_pass");
    if(m_host != "" || m_port != 0) m_params = true; // проверяем, введены ли параметры для MQTT
    else m_params = false;
}

void jeeui2::mqtt_reconnect() {

    if(client.connected() || !mqtt_enable) return;
    mqtt_ok = false;
    client.disconnect();
    client.setServer(m_host.c_str(), m_port);


    // client.setCallback(mqt);
    client.setCallback(mqtt_callback);


    static unsigned long i;
    static int rc = 0;
    if(i + rc > millis() || client.connected()) return;
    i = millis();
    rc = 5000;

    if(dbg)Serial.print("Attempting MQTT connection...");
    if (client.connect(mc.c_str(), m_user.c_str(), m_pass.c_str())) {
        if(dbg)Serial.println("connected");
        if(remoteControl){
            // client.subscribe(id("jee/set/#").c_str());
            client.subscribe(id("jee/get/").c_str());
            // client.publish(id("config").c_str(), config.c_str(), false);
            subscribeAll();
            onConnect();
        }
        mqtt_ok = true; 
    } else {
      if(dbg)Serial.print("failed, rc=");
      if(dbg)Serial.print(client.state());
      if(dbg)Serial.println(" try again in 5 seconds");
      mqtt_ok = false;
    }
}

void jeeui2::mqtt_handle(){
    if (!connected || !m_params || !mqtt_enable) return; 
    mqtt_reconnect();
    client.loop();

    if(_t_inc_current){
        _t_inc_current = false;
        remControl(_t_tpc_current, _t_pld_current);
        _t_tpc_current = "";
        _t_pld_current = "";
    }

}

void mqtt_callback(char* topic, byte* payload, unsigned int len) { 
    String tpc = String(topic);
    tpc = tpc.substring(tpc.indexOf('/') + 1, tpc.length());
    Serial.println("Message [" + tpc + "] ");
    String msg = ""; 
    for (int i= 0; i < len; i++) {
        msg += (char)payload[i];
    }
    
    if(tpc == "jee/get/" || tpc.indexOf("jee/set/") != -1){
        tpc = tpc.substring(4, tpc.length());
        _t_tpc_current = tpc;
        _t_pld_current = msg;
        _t_inc_current = true;
    }
    else mqt(tpc, msg);

}

void jeeui2::remControl(String topic, String payload){

    if(dbg)Serial.println("RC [" + topic + " - " + payload + "]");
    if(topic == "get/") client.publish(id("jee/cfg").c_str(), config.c_str(), false);
    if(topic.indexOf("set/") != -1){
        topic = topic.substring(4, topic.length());
        if(dbg) Serial.println("SET: " + topic);
        var(topic, payload);
        as();
    }
}

void jeeui2::publish(String topic, String payload, bool retained){
    if (!connected || !m_params || !mqtt_enable) return; 
    client.publish(id(topic).c_str(), payload.c_str(), retained);
    // if(dbg)Serial.println("Publish: [" + id(topic) + " - " + payload + "]");
}

void jeeui2::publish(String topic, String payload){
    if (!connected || !m_params || !mqtt_enable) return; 
    publish(topic, payload, false);
}

void jeeui2::subscribe(String topic){
    if (!connected || !m_params || !mqtt_enable) return; 
    client.subscribe(id(topic).c_str());
}

void jeeui2::subscribeAll(){

    DynamicJsonDocument doc(10000);
    deserializeJson(doc, config);
    JsonObject root = doc.as<JsonObject>();
    for (JsonPair kv : root) {
        String key = String(kv.key().c_str());
        if( 
            key != "wifi"       &&
            key != "m_pref"     &&
            key != "m_host"     &&
            key != "m_port"     &&
            key != "m_user"     &&
            key != "m_pass"     &&
            key != "ssid"       &&
            key != "pass"       &&
            key != "ap_ssid"    &&
            key != "ap_pass"
            ){
            if(dbg)Serial.print(kv.key().c_str()); Serial.print(" --- "); Serial.println(kv.value().as<char*>());
            client.subscribe(id("jee/set/" + String(kv.key().c_str())).c_str());
        }
    }
}