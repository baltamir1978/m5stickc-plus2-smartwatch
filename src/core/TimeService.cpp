#include "core/TimeService.h"
#include <cstdio>
#include <cstring>

namespace {

// Día de la semana con el algoritmo de Sakamoto. Devuelve 0=Domingo .. 6=Sábado.
int computeWeekday(int y, int m, int d) {
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3) y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

// Rellena dt con la fecha/hora del momento de compilación (__DATE__ / __TIME__).
bool parseBuildDateTime(m5::rtc_datetime_t& dt) {
  char mon[4] = {0};
  int day = 1, year = 2026, h = 0, mi = 0, s = 0;
  if (sscanf(__DATE__, "%3s %d %d", mon, &day, &year) != 3) return false;
  if (sscanf(__TIME__, "%d:%d:%d", &h, &mi, &s) != 3) return false;

  static const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char* p = strstr(months, mon);
  int month = p ? static_cast<int>((p - months) / 3 + 1) : 1;

  dt.date.year    = year;
  dt.date.month   = month;
  dt.date.date    = day;
  dt.date.weekDay = computeWeekday(year, month, day);
  dt.time.hours   = h;
  dt.time.minutes = mi;
  dt.time.seconds = s;
  return true;
}

}  // namespace

void TimeService::begin() {
  _prefs.begin("timesvc", false);
  _lastSyncYmd = _prefs.getUInt("lastsync", 0);
  _neverSynced = (_lastSyncYmd == 0);

  auto dt = M5.Rtc.getDateTime();
  if (dt.date.year < 2023) {  // RTC sin hora válida tras arranque en frío
    m5::rtc_datetime_t build{};
    if (parseBuildDateTime(build)) {
      M5.Rtc.setDateTime(build);
    }
  }
}

int TimeService::todayYmd() {
  LocalTime t = now();
  return t.year * 10000 + t.month * 100 + t.day;
}

bool TimeService::needsSync() {
  LocalTime t = now();
  int today = t.year * 10000 + t.month * 100 + t.day;
  if (static_cast<int>(_lastSyncYmd) == today) return false;  // ya sincronizado hoy
  if (_neverSynced) return true;                              // primera conexión
  return t.hour >= 4;                                         // re-sync diario desde las 04:00
}

void TimeService::applyPhoneTime(const LocalTime& t) {
  m5::rtc_datetime_t dt{};
  dt.date.year    = t.year;
  dt.date.month   = t.month;
  dt.date.date    = t.day;
  dt.date.weekDay = computeWeekday(t.year, t.month, t.day);
  dt.time.hours   = t.hour;
  dt.time.minutes = t.minute;
  dt.time.seconds = t.second;
  M5.Rtc.setDateTime(dt);
  markSynced();
}

void TimeService::markSynced() {
  _lastSyncYmd = static_cast<uint32_t>(todayYmd());
  _neverSynced = false;
  _prefs.putUInt("lastsync", _lastSyncYmd);
}

LocalTime TimeService::now() {
  auto dt = M5.Rtc.getDateTime();
  LocalTime t;
  t.year    = dt.date.year;
  t.month    = dt.date.month;
  t.day     = dt.date.date;
  t.weekday = dt.date.weekDay;
  t.hour    = dt.time.hours;
  t.minute  = dt.time.minutes;
  t.second  = dt.time.seconds;
  return t;
}

const char* TimeService::weekdayShort(int wd) {
  static const char* w[] = {"DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"};
  if (wd < 0 || wd > 6) return "---";
  return w[wd];
}

const char* TimeService::monthShort(int m) {
  static const char* mo[] = {"ENE", "FEB", "MAR", "ABR", "MAY", "JUN",
                             "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"};
  if (m < 1 || m > 12) return "---";
  return mo[m - 1];
}
