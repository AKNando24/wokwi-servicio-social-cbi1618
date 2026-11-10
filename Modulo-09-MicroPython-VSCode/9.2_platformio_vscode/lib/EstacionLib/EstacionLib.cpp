#include "EstacionLib.h"

FiltroMedia::FiltroMedia() { reiniciar(); }

void FiltroMedia::reiniciar() {
  _cuenta = 0;
  _indice = 0;
  _suma = 0;
  for (uint8_t i = 0; i < TAMANO; i++) _muestras[i] = 0;
}

void FiltroMedia::agregar(float valor) {
  _suma -= _muestras[_indice];        // saca la lectura más antigua
  _muestras[_indice] = valor;
  _suma += valor;
  _indice = (_indice + 1) % TAMANO;   // buffer circular
  if (_cuenta < TAMANO) _cuenta++;
}

float FiltroMedia::promedio() const { return _cuenta ? _suma / _cuenta : 0; }

bool FiltroMedia::lleno() const { return _cuenta == TAMANO; }

Umbral::Umbral(float histeresis) : _histeresis(histeresis), _activa(false) {}

bool Umbral::evaluar(float valor, float limite) {
  if (!_activa && valor >= limite) _activa = true;
  else if (_activa && valor <= limite - _histeresis) _activa = false;
  return _activa;
}

bool Umbral::activa() const { return _activa; }
