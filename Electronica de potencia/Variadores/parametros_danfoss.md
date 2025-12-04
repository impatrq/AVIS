# Parámetros Basicos Modo Manual

| Parámetro | Valor   | Descripción                  |
|-----------|---------|------------------------------|
| P013      | 1       |                              |
| P002      | 1       | Modo local                   |
| P003      |         | Frecuencia en modo local     |

---

## Parametros basicos modo remoto 

| Parámetro | Valor   | Descripción                  |
|-----------|---------|------------------------------|
| P002      | 0       | Modo remoto                  |
| P200      | 1       | Ambos sentidos               |
| P102      |         | Potencia de motor (datos)    |
| P103      |         | Tensión de motor             |
| P104      |         | Frecuencia de motor          |
| P105      |         | Corriente de motor           |
| P106      |         | Velocidad de motor           |
| P107      | AMA *   |                              |
| P205      | 50,000Hz| Velocidad máx. en 50Hz       |
| P206      | 2       | Rampa senoidal               |
| P207      | 3       | Rampa de aceleración         |
| P208      | 3       | Rampa de desaceleración      |
| P215      | **      | Referencia en modo remoto    |

---

## Arranques de entradas digitales

| Parámetro | Valor | Descripción                   |
|-----------|-------|-------------------------------|
| P302      | 7     | Arranque en sentido horario   |
| P303      | 10    | Arranque en sentido antihorario |

---

## Notas

- * Al poner el parámetro **P107 = 2**, el equipo entra en modo de **autotuning**, calculando la impedancia del motor.  
  Al oprimir la marcha local, comienza a zumbar sin mover el eje del motor y después de unos segundos termina.

- ** El valor que se carga en el parámetro **P215** es porcentual (0 a 100%) respecto al rango determinado por los parámetros **P204–P205**.  
  Ejemplo: si **P205 = 50Hz**, **P204 = 0**, y **P215 = 10%**, la velocidad resultante es **5Hz**.
