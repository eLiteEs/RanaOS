// kernel/date_wrapper.cpp

#include "date.h"

extern "C" {

uint8_t get_second() {
    return getSecond();
}

uint8_t get_minute() {
    return getMinute();
}

uint8_t get_hour() {
    return getHour();
}

uint8_t get_day() {
    return getDay();
}

uint8_t get_month() {
    return getMonth();
}

uint8_t get_year() {
    return getYear();
}

const char* get_weekday_nme() {
    return get_weekday_name();
}

}
