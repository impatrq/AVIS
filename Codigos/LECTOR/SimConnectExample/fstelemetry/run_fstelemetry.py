from fstelemetry.telemetry import Telemetry

# Solo registramos el pitch
keys = ["PLANE_BANK_DEGREES","PLANE_PITCH_DEGREES"]

# Crear el objeto Telemetry con esa única clave
t = Telemetry(keys)

# Ejecutar la escucha con intervalo de segundos
t.listen(path="datos.csv", interval=0.3)
