#include "JeeUI2.h"

bool _t_inc_current = false;
String _t_tpc_current;
String _t_pld_current;
String _t_prf_current;
bool _t_remotecontrol;

String _m_host;
int _m_port;
String _m_user;
String _m_pass;
bool _m_params;

void jeeui2::connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}
       
String jeeui2::id(String topic){
    if(m_pref == "") return topic;
    else return m_pref + '/' + topic;
}

bool mqtt_connected = false;
bool mqtt_connect = false;





void jeeui2::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void jeeui2::onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void jeeui2::onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

typedef void (*mqttCallback) (String topic, String payload);
mqttCallback mqt;

void fake(){}
void emptyFunction(String, String){}

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), bool remotecontrol){
    
    mqtt_enable = true;
    if(param(F("m_pref")) == F("null")) var(F("m_pref"), pref);
    if(param(F("m_host")) == F("null")) var(F("m_host"), host);
    if(param(F("m_port")) == F("null")) var(F("m_port"), String(port));
    if(param(F("m_user")) == F("null")) var(F("m_user"), user);
    if(param(F("m_pass")) == F("null")) var(F("m_pass"), pass);

    mqtt_update();

    _t_prf_current = m_pref;
    if (remotecontrol) _t_remotecontrol = true;
    mqt = mqttFunction;

}

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload)){
    mqtt(pref, host, port, user, pass, mqttFunction, false);
    onConnect = fake;
}

void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload)){
    getAPmac();
    mqtt(mc, host, port, user, pass, mqttFunction, false);
    onConnect = fake;
}

void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), bool remotecontrol){
    getAPmac();
    mqtt(mc, host, port, user, pass, mqttFunction, remotecontrol);
    onConnect = fake;
}

void jeeui2::mqtt(String host, int port, String user, String pass, bool remotecontrol){
    getAPmac();
    mqtt(mc, host, port, user, pass, emptyFunction, remotecontrol);
    onConnect = fake;
}

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, bool remotecontrol){
    getAPmac();
    mqtt(pref, host, port, user, pass, emptyFunction, remotecontrol);
    onConnect = fake;
}

void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) (), bool remotecontrol){
    getAPmac();
    onConnect = mqttConnect;
    mqtt(pref, host, port, user, pass, emptyFunction, remotecontrol);

}
void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) ()){
    getAPmac();
    onConnect = mqttConnect;
    mqtt(pref, host, port, user, pass, mqttFunction, false);
}
void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) ()){
    getAPmac();
    onConnect = mqttConnect;
    mqtt(mc, host, port, user, pass, mqttFunction, false);
}
void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) (), bool remotecontrol){
    getAPmac();
    onConnect = mqttConnect;
    mqtt(mc, host, port, user, pass, mqttFunction, remotecontrol);
}

void jeeui2::mqtt_update(){
    if(!mqtt_enable) return;
    m_pref = param(F("m_pref"));
    m_host = param(F("m_host"));
    m_port = param(F("m_port")).toInt();
    m_user = param(F("m_user"));
    m_pass = param(F("m_pass"));
    if(m_host != "" && m_port != 0) m_params = true; // проверяем, введены ли параметры для MQTT
    else m_params = false;
}

void jeeui2::pub_mqtt(String key, String value){
    for(int i = 0; i < pub_num; i++){
        if(key == pub_id[i]) publish(pub_id[i], value, false);
        if(dbg)Serial.println("pub: [" + key + " - " + value + "]");
    }
}

void jeeui2::publish(String topic, String payload, bool retained){
    if (!connected || !m_params || !mqtt_enable) return; 
    mqttClient.publish(id(topic).c_str(), 0, retained, payload.c_str());
}


bool first_connect = true;

void jeeui2::mqtt_handle(){
    static bool st = false;
    if(!st){
        st = true;
        _m_host = param("m_host");
        _m_port = param("m_port").toInt();
        _m_user = param("m_user");
        _m_pass = param("m_pass");
        mqttClient.onConnect(_onMqttConnect);
        mqttClient.onDisconnect(onMqttDisconnect);
        mqttClient.onSubscribe(onMqttSubscribe);
        mqttClient.onUnsubscribe(onMqttUnsubscribe);
        mqttClient.onMessage(onMqttMessage);
        mqttClient.onPublish(onMqttPublish);
        mqttClient.setCredentials(_m_user.c_str(), _m_pass.c_str());
        mqttClient.setServer(_m_host.c_str(), _m_port);
    }

    if(mqtt_connect) onMqttConnect();
    check_wifi_state();
    remControl();
}

void jeeui2::check_wifi_state(){
    static bool old_wifi_connected = false;
    static unsigned long i;
    // if(old_wifi_connected == connected) return;

    if(i + 5000 > millis()) return;
    i = millis();

    // old_wifi_connected = connected;
    if(connected && !mqtt_connected) connectToMqtt();

}

void jeeui2::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT. ---------------------------------------------------------------");
  mqtt_connect = false;
  mqtt_connected = false;

}

void jeeui2::_onMqttConnect(bool sessionPresent) {
    mqtt_connect = true;
    
}

void jeeui2::onMqttConnect(){
    mqtt_connect = false;
    mqtt_connected = true;
    Serial.println("Connected to MQTT.");
    if(_t_remotecontrol){
        subscribeAll();
    }
    
}

void jeeui2::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    Serial.println("Publish received.");
    String tpc = String(topic);
    if(_t_prf_current != "") tpc = tpc.substring(_t_prf_current.length() + 1, tpc.length());
    String msg = ""; 
    for (int i= 0; i < len; i++) {
        msg += (char)payload[i];
    }
    Serial.println(String(F("Message [ pref: ")) + _t_prf_current + " > " + tpc + " -- " + msg + " ] ");
    if(tpc.startsWith(F("jee/get/")) || tpc.startsWith(F("jee/set/"))){
    
        tpc = tpc.substring(4, tpc.length());
        _t_tpc_current = tpc;
        _t_pld_current = msg;
        _t_inc_current = true;

    }
    else mqt(tpc, msg);
}

void jeeui2::remControl(){

    if(!_t_remotecontrol || !_t_inc_current) return;
    _t_inc_current = false;
    
    if(dbg)Serial.println("RC [" + _t_tpc_current + " - " + _t_pld_current + "]");
    if(_t_tpc_current == "get/") publish("jee/cfg", config, false);
    if(_t_tpc_current.indexOf("set/") != -1){
        _t_tpc_current = _t_tpc_current.substring(4, _t_tpc_current.length());
        if(dbg) Serial.println("SET: " + _t_tpc_current);
        var(_t_tpc_current, _t_pld_current);
        as();
    }
    _t_tpc_current = "";
    _t_pld_current = "";
    
}

void jeeui2::subscribeAll(){
    mqttClient.subscribe(id("jee/get/").c_str(), 0);
    DynamicJsonDocument doc(10000);
    deserializeJson(doc, config);
    JsonObject root = doc.as<JsonObject>();
    for (JsonPair kv : root) {
        String key = String(kv.key().c_str());
        if( 
            key != F("wifi" )     &&
            key != F("m_pref")    &&
            key != F("ssid" )     &&
            key != F("pass")      &&
            key != F("ap_ssid")   &&
            key != F("ap_pass")   &&
            key != F("m_host")   &&
            key != F("m_port")   &&
            key != F("m_user")   &&
            key != F("m_pass") 
            ){
            for(int i = 0; i < pub_num + 1; i++){
                if(dbg)Serial.println(id("jee/set/" + String(kv.key().c_str())).c_str());
                if(key != pub_id[i]) mqttClient.subscribe(id("jee/set/" + key).c_str(), 0);
            }
        }
    }
    if(dbg)Serial.println(F("Subscribe All"));
}


// typedef void (*mqttCallback) (String topic, String payload);
// mqttCallback mqt;

// typedef void (*rcCallback) (String topic, String payload);
// rcCallback rctrl;

// typedef void (*emptyFunc) (String topic, String payload);
// emptyFunc eFunc;

// WiFiClient espClient;
// PubSubClient client(espClient);

// void mqtt_callback(char* topic, byte* payload, unsigned int length) ;

// String jeeui2::id(String topic){
//     if(m_pref == "") return topic;
//     else return m_pref + '/' + topic;
// }

// void emptyFunction(String, String){}

// void (jeeui2::*func)(String topic, String payload);

// void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), bool remotecontrol){
    
//     mqtt_enable = true;
//     if(param(F("m_pref")) == F("null")) var(F("m_pref"), pref);
//     if(param(F("m_host")) == F("null")) var(F("m_host"), host);
//     if(param(F("m_port")) == F("null")) var(F("m_port"), String(port));
//     if(param(F("m_user")) == F("null")) var(F("m_user"), user);
//     if(param(F("m_pass")) == F("null")) var(F("m_pass"), pass);

//     mqtt_update();

//     _t_prf_current = m_pref;
//     if (remotecontrol) _t_remotecontrol = true;
//     mqt = mqttFunction;

// }

// void fake(){}

// void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload)){
//     mqtt(pref, host, port, user, pass, mqttFunction, false);
//     onConnect = fake;
// }

// void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload)){
//     getAPmac();
//     mqtt(mc, host, port, user, pass, mqttFunction, false);
//     onConnect = fake;
// }

// void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), bool remotecontrol){
//     getAPmac();
//     mqtt(mc, host, port, user, pass, mqttFunction, remotecontrol);
//     onConnect = fake;
// }

// void jeeui2::mqtt(String host, int port, String user, String pass, bool remotecontrol){
//     getAPmac();
//     mqtt(mc, host, port, user, pass, emptyFunction, remotecontrol);
//     onConnect = fake;
// }

// void jeeui2::mqtt(String pref, String host, int port, String user, String pass, bool remotecontrol){
//     getAPmac();
//     mqtt(pref, host, port, user, pass, emptyFunction, remotecontrol);
//     onConnect = fake;
// }

// void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) (), bool remotecontrol){
//     getAPmac();
//     onConnect = mqttConnect;
//     mqtt(pref, host, port, user, pass, emptyFunction, remotecontrol);

// }
// void jeeui2::mqtt(String pref, String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) ()){
//     getAPmac();
//     onConnect = mqttConnect;
//     mqtt(pref, host, port, user, pass, mqttFunction, false);
// }
// void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) ()){
//     getAPmac();
//     onConnect = mqttConnect;
//     mqtt(mc, host, port, user, pass, mqttFunction, false);
// }
// void jeeui2::mqtt(String host, int port, String user, String pass, void (*mqttFunction) (String topic, String payload), void (*mqttConnect) (), bool remotecontrol){
//     getAPmac();
//     onConnect = mqttConnect;
//     mqtt(mc, host, port, user, pass, mqttFunction, remotecontrol);
// }



// void jeeui2::mqtt_update(){
//     if(!mqtt_enable) return;
//     m_pref = param(F("m_pref"));
//     m_host = param(F("m_host"));
//     m_port = param(F("m_port")).toInt();
//     m_user = param(F("m_user"));
//     m_pass = param(F("m_pass"));
//     if(m_host != "" && m_port != 0) m_params = true; // проверяем, введены ли параметры для MQTT
//     else m_params = false;
// }

// void jeeui2::mqtt_reconnect() {
//     Serial.println("Connecting to MQTT...");
//     mqttClient.connect();

// }

// void jeeui2::mqtt_handle(){
//     if (!connected || !m_params || !mqtt_enable) return; 
//     client.loop();
//     mqtt_reconnect();
//     if(_t_inc_current) remControl();

// }

// void mqtt_callback(char* topic, byte* payload, unsigned int len) { 
//     String tpc = String(topic);
//     if(_t_prf_current != "") tpc = tpc.substring(_t_prf_current.length() + 1, tpc.length());
//     String msg = ""; 
//     for (int i= 0; i < len; i++) {
//         msg += (char)payload[i];
//     }
//     Serial.println(String(F("Message [ pref: ")) + _t_prf_current + " >" + tpc + " -- " + msg + "] ");
//     if(tpc.startsWith(F("jee/get/")) || tpc.startsWith(F("jee/set/"))){
//         tpc = tpc.substring(4, tpc.length());
//         _t_tpc_current = tpc;
//         _t_pld_current = msg;
//         _t_inc_current = true;
//     }
//     else mqt(tpc, msg);

// }

// void jeeui2::remControl(){
//     if(!_t_remotecontrol) return;
//     if(dbg)Serial.println("RC [" + _t_tpc_current + " - " + _t_pld_current + "]");
//     if(_t_tpc_current == "get/") client.publish(id("jee/cfg").c_str(), config.c_str(), false);
//     if(_t_tpc_current.indexOf("set/") != -1){
//         _t_tpc_current = _t_tpc_current.substring(4, _t_tpc_current.length());
//         if(dbg) Serial.println("SET: " + _t_tpc_current);
//         var(_t_tpc_current, _t_pld_current);
//         as();
//     }
//     _t_tpc_current = "";
//     _t_pld_current = "";
//     _t_inc_current = false;
// }

// void jeeui2::publish(String topic, String payload, bool retained){
//     if (!connected || !m_params || !mqtt_enable) return; 
//     client.publish(id(topic).c_str(), payload.c_str(), retained);
//     // if(dbg)Serial.println("Publish: [" + id(topic) + " - " + payload + "]");
// }

// void jeeui2::publish(String topic, String payload){
//     if (!connected || !m_params || !mqtt_enable) return; 
//     publish(topic, payload, false);
// }

// void jeeui2::subscribe(String topic){
//     if (!connected || !m_params || !mqtt_enable) return; 
//     client.subscribe(id(topic).c_str());
// }

// void jeeui2::subscribeAll(){

//     DynamicJsonDocument doc(10000);
//     deserializeJson(doc, config);
//     JsonObject root = doc.as<JsonObject>();
//     for (JsonPair kv : root) {
//         String key = String(kv.key().c_str());
//         if( 
//             key != F("wifi" )     &&
//             key != F("m_pref")    &&
//             key != F("ssid" )     &&
//             key != F("pass")      &&
//             key != F("ap_ssid")   &&
//             key != F("ap_pass")   &&
//             key != F("m_host")   &&
//             key != F("m_port")   &&
//             key != F("m_user")   &&
//             key != F("m_pass") 
//             ){
//             for(int i = 0; i < pub_num + 1; i++){
//                 if(dbg)Serial.println(id("jee/set/" + String(kv.key().c_str())).c_str());
//                 if(key != pub_id[i]) client.subscribe(id("jee/set/" + key).c_str());
//             }
//         }
//     }
//     if(dbg)Serial.print(F("Subscribe All"));
// }

// void jeeui2::pub_mqtt(String key, String value){
//     for(int i = 0; i < pub_num; i++){
//         if(key == pub_id[i]) publish(pub_id[i], value, false);
//         if(dbg)Serial.println("pub: [" + key + " - " + value + "]");
//     }
// }