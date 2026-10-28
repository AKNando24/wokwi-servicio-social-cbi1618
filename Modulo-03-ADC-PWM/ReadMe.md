# Módulo 3: Conversión Analógica-Digital (ADC) y PWM

## Fundamentos Teóricos e Ingeniería de Señales
1. **Conversión ADC en ATmega328P:**
   * **Resolución:** 10 bits ($2^{10} = 1024$ niveles, valores de 0 a 1023).
   * **Tensión de Referencia:** $V_{ref} = 5\text{V}$, resolución por bit $\approx 4.88\text{mV}$.
2. **Modelo Matemático NTC (Ecuación del parámetro $\beta$):**
   $$\frac{1}{T} = \frac{1}{T_0} + \frac{1}{\beta}\ln\left(\frac{R}{R_0}\right)$$
3. **Acondicionamiento Digital (Filtro Exponencial EMA):**
   $$y[k] = \alpha \cdot x[k] + (1 - \alpha) \cdot y[k-1]$$
   Donde $\alpha = 0.15$ atenúa picos de ruido sin introducir latencia crítica.
4. **Modulación por Ancho de Pulsos (PWM):**
   * Frecuencia de ciclo de trabajo a 8 bits (0 a 255) controlando potencia entregada a actuadores y cargas pasivas.