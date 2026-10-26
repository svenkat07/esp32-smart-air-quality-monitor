#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "monitor.hpp"
#include "secrets.h"

constexpr int DHT_PIN=4,GAS_PIN=34,LED_PIN=25;
DHT dht(DHT_PIN,DHT22);
QueueHandle_t latest;
// Only networkTask owns these objects; no cross-task MQTT calls.
WiFiClient transport;
PubSubClient mqtt(transport);
void sensorTask(void*) {
  air::Monitor monitor;TickType_t wake=xTaskGetTickCount();
  for(;;){
    air::Sample s{millis(),dht.readTemperature(),dht.readHumidity(),int(analogReadMilliVolts(GAS_PIN)),false};
    auto result=monitor.update(s);
    digitalWrite(LED_PIN,result.state==air::State::alert||result.state==air::State::fault);
    xQueueOverwrite(latest,&result);
    vTaskDelayUntil(&wake,pdMS_TO_TICKS(2000));
  }
}
void networkTask(void*) {
  WiFi.mode(WIFI_STA);WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  mqtt.setServer(MQTT_HOST,MQTT_PORT);mqtt.setSocketTimeout(2);
  configASSERT(mqtt.setBufferSize(512));
  uint32_t lastTry=millis()-5000,lastWifi=millis(),lastSent=millis()-2000;
  air::Result pending{};bool have=false;char clientId[40];
  snprintf(clientId,sizeof(clientId),"venkat-air-%llx",(unsigned long long)ESP.getEfuseMac());
  for(;;){
    air::Result incoming;
    if(xQueueReceive(latest,&incoming,0)==pdPASS){pending=incoming;have=true;}
    const uint32_t now=millis();
    if(WiFi.status()!=WL_CONNECTED){
      if(now-lastWifi>=10000){WiFi.reconnect();lastWifi=now;}
    }else{
      if(!mqtt.connected()&&now-lastTry>=5000){
        lastTry=now;
        if(mqtt.connect(clientId,"venkat/air/status",0,true,"offline"))
          mqtt.publish("venkat/air/status","online",true);
      }
      if(mqtt.connected()){
        mqtt.loop();
        if(have&&now-lastSent>=2000){
          // Never republish a sample more than 10 seconds old.
          if(uint32_t(now-pending.sample.ms)>10000)have=false;
          else{
            char payload[320];int n=air::json(pending,payload,sizeof(payload));
            if(n>0&&n<int(sizeof(payload))&&mqtt.publish("venkat/air/telemetry",payload,false)){
              have=false;lastSent=now;
            }
          }
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
void setup(){
  Serial.begin(115200);pinMode(LED_PIN,OUTPUT);dht.begin();
  analogReadResolution(12);analogSetPinAttenuation(GAS_PIN,ADC_11db);
  latest=xQueueCreate(1,sizeof(air::Result));configASSERT(latest);
  configASSERT(xTaskCreate(sensorTask,"sensor",4096,nullptr,2,nullptr)==pdPASS);
  configASSERT(xTaskCreate(networkTask,"network",6144,nullptr,1,nullptr)==pdPASS);
}
void loop(){vTaskDelay(pdMS_TO_TICKS(1000));}
