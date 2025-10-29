import csv
import time
import serial
import serial.tools.list_ports

ruta_archivo = 'datos.csv'
puerto_usb = 'COM15'   # Puerto asignado a la Pico (puede cambiar al reconectar)
baudrate = 115200      # Debe coincidir con el usado por la Pico

def conectar_pico():
    """Intenta conectar con la Pico y esperar el mensaje de inicio."""
    global puerto_usb
    while True:
        try:
            print(f"🔌 Buscando Pico en {puerto_usb}...")
            ser = serial.Serial(puerto_usb, baudrate, timeout=0.5)
            time.sleep(2)  # Esperar inicialización USB

            print("⏳ Esperando mensaje de conexión de la Pico...")
            while True:
                try:
                    linea = ser.readline().decode('utf-8').strip()
                    if linea:
                        print("📥", linea)
                        if "Pico lista" in linea:
                            print("✅ Conexión establecida con la Pico.")
                            return ser
                except serial.SerialException:
                    break  # Si se desconecta en este punto, volver a intentar
                time.sleep(0.1)

        except serial.SerialException:
            print("❌ Pico no detectada, esperando reconexión...")
            time.sleep(2)
            continue


def leer_datos_y_enviar(ser):
    """Lee el archivo CSV y envía los datos de Bank y Pitch a la Pico."""
    with open(ruta_archivo, newline='', mode='r') as archivo:
        lector = csv.reader(archivo)
        encabezado = next(lector)

        # --- Verificación de columnas ---
        columnas_necesarias = ['time', 'PLANE_BANK_DEGREES', 'PLANE_PITCH_DEGREES']
        for col in columnas_necesarias:
            if col not in encabezado:
                print(f"❌ Falta columna requerida: {col}")
                raise SystemExit

        # Índices de columnas
        i_time = encabezado.index('time')
        i_bank = encabezado.index('PLANE_BANK_DEGREES')
        i_pitch = encabezado.index('PLANE_PITCH_DEGREES')

        print("✅ Lectura iniciada: mostrando Bank (Roll) y Pitch en tiempo real")

        while True:
            try:
                fila = next(lector, None)
                if fila is None:
                    time.sleep(0.5)
                    continue

                if len(fila) < len(encabezado):
                    print(f"❌ Fila malformada o incompleta: {fila}")
                    continue

                timestamp = float(fila[i_time])
                bank = float(fila[i_bank])
                pitch = float(fila[i_pitch])

                # --- Mensaje formateado para enviar a la Pico ---
                mensaje = f"{timestamp:.2f} | Roll: {bank:.6f} | Pitch: {pitch:.6f}"
                

                # Enviar al microcontrolador
                ser.write((mensaje + '\n').encode('utf-8'))

                # Leer respuesta desde la Pico
                respuesta = ser.readline().decode('utf-8').strip()
                if respuesta:
                    print("📥", respuesta)

            except (serial.SerialException, OSError):
                print("⚠️ Conexión con la Pico perdida. Esperando reconexión...")
                ser.close()
                return  # Volver al bucle principal para reconectar
            except ValueError:
                print(f"❌ Error de conversión en fila: {fila}")


# --- Bucle principal persistente ---
while True:
    try:
        ser = conectar_pico()       # Espera hasta que la Pico se conecte
        leer_datos_y_enviar(ser)    # Envía los datos
    except FileNotFoundError:
        print(f"❌ No se encontró el archivo en la ruta: {ruta_archivo}")
        time.sleep(3)
    except Exception as e:
        print(f"⚠️ Error inesperado: {e}")
        time.sleep(3)
