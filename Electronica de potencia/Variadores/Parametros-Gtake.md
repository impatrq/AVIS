# Parámetros adicionales

## Bloque B

| Parámetro | Valor | Descripción        |
|-----------|-------|--------------------|
| Bo-01     | 3     | Potenciómetro      |
| B1-00     | 1     | Control por bornera|
| B2-00     | ?     |                    |

---

## Bloque C

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| C0-01     | 3     | FWD         |
| C0-02     | 4     | REV         |
| C0-03     | 9?    |             |

---

## Bloque D

| Parámetro | Valor | Descripción                                                                 |
|-----------|-------|-----------------------------------------------------------------------------|
| D0-00     | 1     | Motor de frecuencia variable                                                |
| D0-01     | *     | Potencia del motor (KW kilowatts)                                           |
| D0-02     | *     | Tensión nominal del motor (V voltaje)                                       |
| D0-03     | *     | Corriente nominal del motor (A amperaje)                                    |
| D0-04     | *     | Frecuencia nominal del motor (Hz)                                           |
| D0-06     | *     | Velocidad nominal en RPM                                                    |
| D0-22     | 1     | Autotuning estático (se realiza una única vez, en caso de reinicio o fallo) |
| D1-00     | 0     | V/f lineal                                                                  |

---

## Notas

- En caso de modificar algunos parámetros, se recomienda **anotarlos**, ya que es fácil olvidar cuáles se cambiaron.  
- Si se pierden los valores o no se entiende qué se modificó, es más sencillo **reiniciar con el parámetro A0-03** y volver a recorrer la lista anterior.
