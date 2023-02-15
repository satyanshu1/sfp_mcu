#include <time.h>


void setup_time(){
  // initialize timestamp
  configTime(19800, 0, "in.pool.ntp.org");  // gmt_offset_sec=19800 (GMT+5:30), daylight_offset_sec=0, ntp_server to get time from
}

void update_time(){
    struct tm time_info;
    getLocalTime(&time_info);
    strftime (CUR_TIME,50,"%Y-%m-%dT%H:%M:%S.000000Z", &time_info);
}