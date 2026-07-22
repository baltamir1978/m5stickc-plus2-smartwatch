#pragma once
#include <cstring>

// Estado de "llamada entrante". Lo actualiza la capa BLE (ANCS) y lo consume la UI.
class CallAlert {
public:
  void incoming(const char* caller) {
    _active = true;
    if (caller && *caller) {
      strncpy(_caller, caller, sizeof(_caller) - 1);
      _caller[sizeof(_caller) - 1] = '\0';
    } else {
      strcpy(_caller, "Llamada entrante");
    }
  }

  void clear() {
    _active = false;
    _caller[0] = '\0';
  }

  bool        active() const { return _active; }
  const char* caller() const { return _caller; }

private:
  bool _active = false;
  char _caller[40] = {0};
};
