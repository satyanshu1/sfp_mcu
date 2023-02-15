#include <time.h>
#include <WiFi.h>
bool time_synced = false;


bool update_time(){
    if(time_synced==false)
    {
        if(WiFi.status() == WL_CONNECTED)
        {
            configTime(19800, 0, "in.pool.ntp.org");  // gmt_offset_sec=19800 (GMT+5:30), daylight_offset_sec=0, ntp_server to get time from
            time_synced = true;
        }
    }
    if(time_synced)
    {
        struct tm time_info;
        getLocalTime(&time_info);
        strftime (CUR_TIME,50,"%Y-%m-%dT%H:%M:%S.000000Z", &time_info);
        return true;
    }
    else
        return false;
}