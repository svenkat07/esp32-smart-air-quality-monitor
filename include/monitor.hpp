#pragma once
#include <cmath>
#include <cstdint>
#include <cstdio>

namespace air {
struct Sample { uint32_t ms; float temperature, humidity; int gasMv; bool simulated; };
enum class State { warmup, normal, alert, fault };
inline const char* name(State s) {
  switch(s) { case State::warmup:return "warmup"; case State::normal:return "normal";
    case State::alert:return "alert"; default:return "sensor_fault"; }
}
struct Result { Sample sample; State state; float gasFiltered; };
class Monitor {
  bool initialized_=false, latched_=false, startupComplete_=false;
  float filtered_=0;
public:
  // Demonstration voltage thresholds, not ppm, AQI, or safety limits.
  Result update(Sample s) {
    bool valid=std::isfinite(s.temperature)&&std::isfinite(s.humidity)
      &&s.temperature>=-40&&s.temperature<=80&&s.humidity>=0&&s.humidity<=100
      &&s.gasMv>=0&&s.gasMv<3000;
    if(!valid) return {s,State::fault,filtered_};
    filtered_=initialized_ ? 0.25f*s.gasMv+0.75f*filtered_ : float(s.gasMv);
    initialized_=true;
    if(s.ms>=60000)startupComplete_=true;
    if(!startupComplete_) return {s,State::warmup,filtered_};
    if(filtered_>=1600||s.temperature>=35||s.humidity>=80)latched_=true;
    else if(filtered_<=1300&&s.temperature<=32&&s.humidity<=75)latched_=false;
    return {s,latched_?State::alert:State::normal,filtered_};
  }
};
inline int json(const Result& r,char* out,size_t capacity) {
  char temp[24],hum[24];
  if(std::isfinite(r.sample.temperature))std::snprintf(temp,sizeof(temp),"%.2f",r.sample.temperature);
  else std::snprintf(temp,sizeof(temp),"null");
  if(std::isfinite(r.sample.humidity))std::snprintf(hum,sizeof(hum),"%.2f",r.sample.humidity);
  else std::snprintf(hum,sizeof(hum),"null");
  return std::snprintf(out,capacity,
    "{\"uptime_ms\":%u,\"temperature_c\":%s,\"humidity_pct\":%s,\"gas_adc_mv\":%d,\"gas_filtered_mv\":%.2f,\"state\":\"%s\",\"simulated\":%s}",
    unsigned(r.sample.ms),temp,hum,r.sample.gasMv,r.gasFiltered,name(r.state),r.sample.simulated?"true":"false");
}
}
