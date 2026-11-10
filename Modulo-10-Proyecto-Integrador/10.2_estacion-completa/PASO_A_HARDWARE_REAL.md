# Del simulador al circuito físico

La simulación valida la lógica y las conexiones, pero no reproduce el ruido eléctrico, las tolerancias ni los límites de alimentación. Lista de verificación para llevar la estación al mundo real.

## Antes de armar
- [ ] **Lista de materiales:** ESP32 DevKit, DHT22 (o módulo con resistencia incluida), fotorresistencia, resistencia fija de 10 kΩ para su divisor, OLED SSD1306 I2C, 2 botones, 1 LED, 1 resistencia de 220 Ω, protoboard, cables, cable USB de datos.
- [ ] Revisa que el cable USB sea de **datos** (no solo de carga).

## Voltajes y señales
- [ ] El ESP32 trabaja a **3,3 V** en sus pines. Alimenta sensores y pantalla a 3,3 V o usa convertidores de nivel.
- [ ] **DHT22 suelto:** pull-up de 10 kΩ entre DATA y VCC (los módulos en tarjeta ya la traen).
- [ ] **LDR:** necesita un divisor de voltaje con una resistencia fija; el módulo de Wokwi ya lo trae integrado.
- [ ] **Botones:** el modo `INPUT_PULLUP` funciona igual; agrega, si hace falta, un capacitor de 100 nF contra el rebote.
- [ ] **LED:** conserva la resistencia de 220 Ω.

## Pines del ESP32
- [ ] Usa **ADC1** (GPIO32 a 39) para lecturas analógicas: con Wi-Fi activo, **ADC2 no funciona**.
- [ ] Evita los pines de arranque (GPIO0, 2, 12 y 15) para botones o sensores: pueden impedir que la placa inicie.
- [ ] GPIO34 a 39 son solo entrada y no tienen pull-up interno.

## Alimentación
- [ ] El Wi-Fi genera picos de consumo. Una fuente USB débil provoca reinicios aleatorios: usa una fuente de al menos 1 A.
- [ ] Añade un capacitor de 100 µF cerca de la alimentación de la placa si ves reinicios.

## Software
- [ ] Cambia `ID_ESTACION` por un identificador único.
- [ ] No dejes contraseñas de Wi-Fi ni claves en el repositorio público: usa un archivo de configuración fuera de Git.
- [ ] Sustituye `Wokwi-GUEST` por tu red real (`WiFi.begin(ssid, clave)` sin el canal fijo).
- [ ] Un broker público es visible para todos: para uso real, usa un broker propio con usuario, contraseña y TLS.
- [ ] Prueba **cada componente por separado** antes de integrar todo.

## Primera puesta en marcha
1. Alimenta solo con USB y revisa el monitor serie a 115200 baudios.
2. Verifica la lectura del DHT22 y del LDR.
3. Verifica la pantalla OLED (dirección I2C 0x3C; si no responde, usa el escáner del Módulo 6).
4. Activa Wi-Fi y MQTT y comprueba los mensajes con un cliente MQTT.
