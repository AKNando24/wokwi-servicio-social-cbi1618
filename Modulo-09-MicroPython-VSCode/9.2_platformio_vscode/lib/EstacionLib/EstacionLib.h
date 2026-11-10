/*
  EstacionLib: biblioteca propia del curso (Módulo 9.2)
  - FiltroMedia: promedio móvil de las últimas N lecturas (sin memoria dinámica).
  - Umbral: comparador con histéresis para evitar que una alarma oscile.
  Servicio Social CBI-1618 - Pérez Sánchez Fernando Alberto
*/
#ifndef ESTACIONLIB_H
#define ESTACIONLIB_H

#include <Arduino.h>

class FiltroMedia {
 public:
  static const uint8_t TAMANO = 8;

  FiltroMedia();
  void agregar(float valor);        // inserta una lectura nueva
  float promedio() const;           // promedio de las lecturas disponibles
  bool lleno() const;               // true cuando ya hay TAMANO lecturas
  void reiniciar();

 private:
  float _muestras[TAMANO];
  uint8_t _cuenta;
  uint8_t _indice;
  float _suma;
};

class Umbral {
 public:
  Umbral(float histeresis);
  bool evaluar(float valor, float limite);   // true = alarma activa
  bool activa() const;

 private:
  float _histeresis;
  bool _activa;
};

#endif
