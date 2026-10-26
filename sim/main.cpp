#include "monitor.hpp"
#include <iostream>
#include <limits>
int main(){
  air::Monitor m;
  for(uint32_t i=0;i<120;++i){
    float t=24.0f+0.5f*std::sin(i*0.2f), h=48.0f+2*std::sin(i*0.1f);
    int gas=900+int(25*std::sin(i*0.4f));
    if(i>=40&&i<65)gas=2050; // Synthetic voltage excursion.
    if(i>=75&&i<85)t=37;
    if(i>=90&&i<94)t=std::numeric_limits<float>::quiet_NaN();
    char line[320];auto r=m.update({i*2000,t,h,gas,true});
    int n=air::json(r,line,sizeof(line));if(n<0||n>=int(sizeof(line)))return 1;
    std::cout<<line<<'\n';
  }
}
