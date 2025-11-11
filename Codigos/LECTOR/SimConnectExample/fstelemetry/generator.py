import csv
import time
import serial
import random

puerto_usb = 'COM15'   # Cambia esto al puerto COM correcto
baudrate = 115200      # Debe coincidir con el usado por la Pico

try:
    ser = serial.Serial(puerto_usb, baudrate, timeout=0.1)
    time.sleep(2)  # Esperar a que se inicie la conexión USB

    print("🔌 Esperando conexión con la Pico...")

    # --- Esperar mensaje de conexión desde la Pico ---
    conectado = False
    while not conectado:
        linea = ser.readline().decode('utf-8').strip()
        if linea:
            print("📥", linea)
            if "Pico lista" in linea:
                conectado = True
                print("✅ Conexión establecida con la Pico.")
        time.sleep(0.1)

    print("✅ Generador de datos iniciado: enviando Bank aleatorio entre -3° y 3°")

    tiempo = 0.0
    while True:
        bank = random.uniform(-3.0, 3.0)
        mensaje = f"{tiempo:.2f} | Bank: {bank:.2f}°"
        print(mensaje)

        # Enviar al microcontrolador
        ser.write((mensaje + '\n').encode('utf-8'))

        # Leer respuesta desde la Pico (eco opcional)
        respuesta = ser.readline().decode('utf-8').strip()
        if respuesta:
            print("📥", respuesta)

        tiempo += 0.1
        time.sleep(0.5)

except serial.SerialException:
    print(f"❌ No se pudo abrir el puerto serial: {puerto_usb}")
except KeyboardInterrupt:
    print("\n🛑 Ejecución interrumpida por el usuario.")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
