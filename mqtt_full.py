import paho.mqtt.client as mqtt
import csv
from datetime import datetime

# ================= MQTT DETAILS =================
BROKER = "broker.hivemq.com"
TOPIC_HR = "women_safety/heartrate"
TOPIC_SPO2 = "women_safety/spo2"

# ================= GLOBAL VARIABLES =================
heart_rate = None
spo2 = None

# ================= MQTT CALLBACKS =================
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker")
    client.subscribe(TOPIC_HR)
    client.subscribe(TOPIC_SPO2)

def on_message(client, userdata, msg):
    global heart_rate, spo2

    value = msg.payload.decode().strip()

    # Heart Rate topic
    if msg.topic == TOPIC_HR and value.isdigit():
        heart_rate = int(value)
        print("Heart Rate:", heart_rate)

    # SpO2 topic
    elif msg.topic == TOPIC_SPO2 and value.isdigit():
        spo2 = int(value)
        print("SpO2:", spo2)

    # Save only when both values are received
    if heart_rate is not None and spo2 is not None:
        time_now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        with open("vitals_data.csv", "a", newline="") as file:
            writer = csv.writer(file)
            writer.writerow([time_now, heart_rate, spo2])

        print("Saved to CSV:", heart_rate, spo2)
        print("-----------------------------")

# ================= MQTT CLIENT =================
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, 1883, 60)

print("Listening for Heart Rate & SpO2 data...")
client.loop_forever()