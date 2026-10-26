#include "monitor.hpp"
#include <cassert>
#include <limits>
#include <string>
int main(){
  using air::State;air::Monitor m;
  assert(m.update({0,24,50,900,true}).state==State::warmup);
  assert(m.update({60000,24,50,900,true}).state==State::normal);
  assert(m.update({62000,35,50,900,true}).state==State::alert);
  assert(m.update({64000,33,50,900,true}).state==State::alert);
  assert(m.update({66000,32,50,900,true}).state==State::normal);
  for(int i=0;i<12;++i)m.update({uint32_t(68000+i*2000),24,50,2100,true});
  assert(m.update({92000,24,50,2100,true}).state==State::alert);
  for(int i=0;i<12;++i)m.update({uint32_t(94000+i*2000),24,50,900,true});
  assert(m.update({118000,24,50,900,true}).state==State::normal);
  assert(m.update({120000,24,101,900,true}).state==State::fault);
  auto bad=m.update({122000,std::numeric_limits<float>::quiet_NaN(),50,900,true});
  assert(bad.state==State::fault);char b[320];air::json(bad,b,sizeof(b));
  assert(std::string(b).find("\"temperature_c\":null")!=std::string::npos);
  assert(m.update({124000,24,50,3100,true}).state==State::fault);
  assert(m.update({126000,24,50,900,true}).state==State::normal);
  // Uptime wrapping must not put a running monitor back into startup.
  assert(m.update({0xffffffffu,24,50,900,true}).state==State::normal);
  assert(m.update({1000,24,50,900,true}).state==State::normal);
}
