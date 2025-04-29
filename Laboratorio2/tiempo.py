import serial
import time

# Configura el puerto serial
puerto_serial = 'COM9'
velocidad = 115200

ser = serial.Serial(puerto_serial, velocidad)
time.sleep(2)  # Espera que Arduino reinicie

with open('datos_serial.txt', 'w') as archivo:
    try:
        while True:
            if ser.in_waiting > 0:
                datos = ser.readline().decode('utf-8', errors='ignore').strip()
                print(datos)
                if datos == "Secuencia completada.":
                    print("Terminando captura automática.")
                    break
                archivo.write(datos + '\n')
                archivo.flush()  # Para que no pierdas nada si se interrumpe
    except KeyboardInterrupt:
        print("Lectura interrumpida manualmente.")
    finally:
        ser.close()
        print("Puerto serial cerrado.")
