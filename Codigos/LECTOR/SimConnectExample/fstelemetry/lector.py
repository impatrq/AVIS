import csv
import time
import serial

ruta_archivo = 'datos.csv'
puerto_usb = 'COM3'  # Cambia esto al puerto COM correcto
baudrate = 115200    # Debe coincidir con el usado por la Pico

try:
    ser = serial.Serial(puerto_usb, baudrate, timeout=0.1)
    time.sleep(2)  # Esperar a que se inicie la comunicación USB

    with open(ruta_archivo, newline='', mode='r') as archivo:
        lector = csv.reader(archivo)

        encabezado = next(lector)
        if 'time' not in encabezado or 'PLANE_PITCH_DEGREES' not in encabezado:
            print("❌ Encabezado inválido. Se esperaban las columnas 'time' y 'PLANE_PITCH_DEGREES'.")
        else:
            i_time = encabezado.index('time')
            i_pitch = encabezado.index('PLANE_PITCH_DEGREES')

            while True:
                fila = next(lector, None)
                if fila is None:
                    time.sleep(0.5)
                    continue

                if len(fila) < 2:
                    print(f"❌ Fila malformada o incompleta: {fila}")
                    continue

                try:
                    timestamp = float(fila[i_time])
                    pitch = float(fila[i_pitch])
                    mensaje = f"⏱ {timestamp:.2f} | 🎯 Pitch: {pitch:.6f} "
                    print(mensaje)
                    
                    ser.write((mensaje + '\n').encode('utf-8'))  # Enviar a la Pico

                    # Leer eco
                    respuesta = ser.readline().decode('utf-8').strip()
                    if respuesta:
                        print("📥", respuesta)

                except ValueError:
                    print(f"❌ Error de conversión en fila: {fila}")
except FileNotFoundError:
    print(f"❌ No se encontró el archivo en la ruta: {ruta_archivo}")
except serial.SerialException:
    print(f"❌ No se pudo abrir el puerto serial: {puerto_usb}")
