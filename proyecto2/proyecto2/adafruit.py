# ============================================================
#  Feeds suscritos:
#    servo-base    -> slider 0-180  -> serial: "B<val>\n"
#    servo-hombro  -> slider 0-180  -> serial: "H<val>\n"
#    servo-codo    -> slider 0-180  -> serial: "C<val>\n"
#    servo-garra   -> slider 0-180  -> serial: "G<val>\n"
#    modos         -> "M1" / "M2" / "M3" -> serial: "M1\n" / "M2\n" / "M3\n"
#    grabar        -> "G"  -> serial: "G\n"
#
#  Protocolo serial (9600 baud, 8N1):
#    B<0-180>\n   -> mover servo base
#    H<0-180>\n   -> mover servo hombro
#    C<0-180>\n   -> mover servo codo
#    R<0-180>\n   -> mover servo garra (slider)
#    M1\n         -> modo manual
#    M2\n         -> modo EEPROM (reproducir)
#    M3\n         -> modo UART/Adafruit
#    G\n          -> guardar posición en EEPROM
# ============================================================

import sys
import time
import serial
import serial.tools.list_ports
from Adafruit_IO import MQTTClient

# ──────────────────────────────────────────────
#  CREDENCIALES ADAFRUIT IO
#  Completa con tus datos antes de ejecutar
# ──────────────────────────────────────────────
ADAFRUIT_IO_USERNAME = "Cristianph20"  
ADAFRUIT_IO_KEY      = "aio_aopX40idObFy7eBa51OEdOtPSDcG"  

# ──────────────────────────────────────────────
#  FEEDS
# ──────────────────────────────────────────────
FEED_SERVO_BASE   = "servo-base"
FEED_SERVO_HOMBRO = "servo-hombro"
FEED_SERVO_CODO   = "servo-codo"
FEED_SERVO_GARRA  = "servo-garra"
FEED_MODOS        = "modos"
FEED_GRABAR       = "grabar"

FEEDS_SUSCRITOS = [
    FEED_SERVO_BASE,
    FEED_SERVO_HOMBRO,
    FEED_SERVO_CODO,
    FEED_SERVO_GARRA,
    FEED_MODOS,
    FEED_GRABAR,
]

# ──────────────────────────────────────────────
#  CONFIGURACIÓN SERIAL
# ──────────────────────────────────────────────
SERIAL_PORT = "COM3"    
SERIAL_BAUD = 9600

# ──────────────────────────────────────────────
#  INICIALIZAR SERIAL
# ──────────────────────────────────────────────
def init_serial():
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1)
        time.sleep(2)   # Esperar reset del Arduino al conectar
        print(f"[Serial] Conectado en {SERIAL_PORT} a {SERIAL_BAUD} baud")
        return ser
    except serial.SerialException as e:
        print(f"[Error] No se pudo abrir el puerto serial: {e}")
        print("\nPuertos disponibles:")
        for p in serial.tools.list_ports.comports():
            print(f"  {p.device}  -  {p.description}")
        sys.exit(1)

# ──────────────────────────────────────────────
#  ENVIAR COMANDO AL MICRO
# ──────────────────────────────────────────────
def send_command(ser, cmd):
    if cmd == "0":
        return
    msg = f"{cmd}\n"
    ser.write(msg.encode('utf-8'))
    ser.flush()                  # Asegurar que se envíe inmediatamente
    time.sleep(0.15)             # 150ms — esperar ciclo completo del micro (20ms loop x7)
    print(f"[Serial TX] {cmd}")

# ──────────────────────────────────────────────
#  CALLBACKS ADAFRUIT IO
# ──────────────────────────────────────────────
def connected(client):
    print("[Adafruit] Conectado")
    for feed in FEEDS_SUSCRITOS:
        client.subscribe(feed)
        print(f"[Adafruit] Suscrito a: {feed}")
    print("[Adafruit] Esperando datos del dashboard...")
    # Enviar modo UART automáticamente al conectar
    # para asegurar que el micro esté en el modo correcto
    time.sleep(1)
    send_command(ser, "M3")
    print("[Adafruit] Modo UART enviado al micro")

def disconnected(client):
    print("[Adafruit] Desconectado")
    sys.exit(1)

# ──────────────────────────────────────────────
#  DEBOUNCE DE SLIDER
#  Guarda el último valor enviado por servo.
#  Solo envía si el cambio es mayor al umbral
#  para evitar que el slider inunde de comandos.
# ──────────────────────────────────────────────
ultimo_valor = {
    FEED_SERVO_BASE:   -999,
    FEED_SERVO_HOMBRO: -999,
    FEED_SERVO_CODO:   -999,
    FEED_SERVO_GARRA:  -999,
}
UMBRAL_GRADOS = 3   # Solo enviar si el cambio supera este valor

def ha_cambiado(feed_id, nuevo_val):
    """Retorna True si el cambio supera el umbral y actualiza el registro."""
    global ultimo_valor
    if feed_id not in ultimo_valor:
        return True
    diff = abs(nuevo_val - ultimo_valor[feed_id])
    if diff >= UMBRAL_GRADOS:
        ultimo_valor[feed_id] = nuevo_val
        return True
    return False

def message(client, feed_id, payload):
    print(f"[Adafruit RX] Feed: {feed_id}  |  Valor: {payload}")

    # ── Servos ──
    if feed_id == FEED_SERVO_BASE:
        val = clamp_angle(payload)
        send_command(ser, f"B{val}")

    elif feed_id == FEED_SERVO_HOMBRO:
        val = clamp_angle(payload)
        send_command(ser, f"H{val}")

    elif feed_id == FEED_SERVO_CODO:
        val = clamp_angle(payload)
        send_command(ser, f"C{val}")

    elif feed_id == FEED_SERVO_GARRA:
        val = clamp_angle(payload)
        send_command(ser, f"R{val}")

    # ── Modos ──
    elif feed_id == FEED_MODOS:
        if payload == "M1":
            send_command(ser, "M1")
        elif payload == "M2":
            send_command(ser, "M2")
        elif payload == "M3":
            send_command(ser, "M3")
        elif payload == "0":
            pass
        else:
            print(f"[Advertencia] Modo desconocido: {payload}")

    # ── Grabar ──
    elif feed_id == FEED_GRABAR:
        if payload == "G":
            send_command(ser, "G")
        elif payload == "0":
            pass

def clamp_angle(payload):
    """Convierte el payload a entero y limita al rango 0-180."""
    try:
        val = int(float(payload))
        return max(0, min(180, val))
    except ValueError:
        print(f"[Advertencia] Valor no numérico: {payload}")
        return 90   # Retornar centro si el valor no es válido

# ──────────────────────────────────────────────
#  MAIN
# ──────────────────────────────────────────────
if __name__ == "__main__":

    # Conectar serial al ATmega
    ser = init_serial()

    # Crear cliente MQTT de Adafruit IO
    client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
    client.on_connect    = connected
    client.on_disconnect = disconnected
    client.on_message    = message

    # Conectar a Adafruit IO
    print("[Adafruit] Conectando...")
    client.connect()
    client.loop_background()

    # Loop principal
    try:
        while True:
            # Leer si el micro envía algo (respuestas, confirmaciones)
            if ser.in_waiting > 0:
                linea = ser.readline().decode('utf-8').strip()
                if linea:
                    print(f"[Serial RX] {linea}")
            time.sleep(0.05)

    except KeyboardInterrupt:
        print("\n[Info] Detenido por el usuario")
        ser.close()
        sys.exit(0)