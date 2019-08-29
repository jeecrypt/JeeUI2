#include "JeeUI2.h"

#include <WiFiUdp.h>

WiFiUDP Udp;

void jeeui2::udpBegin(){
    Udp.begin(localUdpPort);
}

void jeeui2::udpLoop(){
    static bool st = false;
    if(!st){
        st = true;
        udpBegin();
    }
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        udpRemoteIP = Udp.remoteIP().toString();
        if(dbg)Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udpRemoteIP.c_str(), Udp.remotePort());
        int len = Udp.read(incomingPacket, 64);
        if (len > 0)
        {
        incomingPacket[len] = 0;
        }
        if(dbg)Serial.printf("UDP packet contents: %s\n", incomingPacket);

        // send back a reply, to the IP address and port we got the packet from
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        //Udp.write(ip.c_str());
        Udp.print(ip.c_str());
        Udp.endPacket();
    }
}