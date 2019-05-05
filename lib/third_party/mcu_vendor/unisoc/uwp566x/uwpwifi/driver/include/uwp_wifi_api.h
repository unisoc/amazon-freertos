#ifndef __UWP_WIFI_API_H
#define __UWP_WIFI_API_H

static inline const char *security2str(int security){
    char *str = NULL;

    switch (security) {
    case WIFI_SECURITY_TYPE_OPEN:
        str = "OPEN";
        break;
    case WIFI_SECURITY_TYPE_PSK:
        str = "WPA/WPA2";
        break;
    default:
        str = "OTHERS";
        break;
    }
    return str;

}

#endif

