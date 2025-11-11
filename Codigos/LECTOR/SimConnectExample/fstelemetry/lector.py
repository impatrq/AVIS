import csv
import time
import serial
import serial.tools.list_ports

ruta_archivo = 'datos.csv'
puerto_usb = 'COM15'
baudrate = 115200


def conectar_pico():
    """Intenta conectar con la Pico y esperar el mensaje de inicio."""
    global puerto_usb
    while True:
        try:
            print(f"🔌 Buscando Pico en {puerto_usb}...")
            ser = serial.Serial(puerto_usb, baudrate, timeout=0.5)
            time.sleep(2)

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
                    break
                time.sleep(0.1)

        except serial.SerialException:
            print("❌ Pico no detectada, esperando reconexión...")
            time.sleep(2)
            continue


def leer_datos_y_enviar(ser):
    """Lee el archivo CSV y envía los datos de Bank y Pitch a la Pico."""
    print("✅ Lectura iniciada desde la última línea del CSV ")

    # Abrir archivo y buscar encabezado
    with open(ruta_archivo, mode='r') as archivo:
        encabezado = archivo.readline().strip().split(',')
        columnas_necesarias = ['time', 'PLANE_BANK_DEGREES', 'PLANE_PITCH_DEGREES']

        for col in columnas_necesarias:
            if col not in encabezado:
                print(f"❌ Falta columna requerida: {col}")
                raise SystemExit

        i_time = encabezado.index('time')
        i_bank = encabezado.index('PLANE_BANK_DEGREES')
        i_pitch = encabezado.index('PLANE_PITCH_DEGREES')

        # Ir al final del archivo y leer solo lo nuevo
        archivo.seek(0, 2)

        while True:
            try:
                linea = archivo.readline()
                if not linea:
                    time.sleep(0.05)
                    continue

                fila = linea.strip().split(',')
                if len(fila) < len(encabezado):
                    continue

                timestamp = float(fila[i_time])
                bank = float(fila[i_bank])
                pitch = float(fila[i_pitch])

                mensaje = f"{timestamp:.2f} | Roll: {bank:.6f} | Pitch: {pitch:.6f}"
                ser.write((mensaje + '\n').encode('utf-8'))

                respuesta = ser.readline().decode('utf-8').strip()
                if respuesta:
                    print("📥", respuesta)

            except (serial.SerialException, OSError):
                print("⚠️ Conexión con la Pico perdida. Esperando reconexión...")
                ser.close()
                return
            except ValueError:
                continue


# --- Bucle principal persistente ---
while True:
    try:
        ser = conectar_pico()
        leer_datos_y_enviar(ser)
    except FileNotFoundError:
        print(f"❌ No se encontró el archivo en la ruta: {ruta_archivo}")
        time.sleep(3)
    except Exception as e:
        print(f"⚠️ Error inesperado: {e}")
        time.sleep(3)
