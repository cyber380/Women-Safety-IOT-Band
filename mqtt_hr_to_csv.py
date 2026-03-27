import paho.mqtt.client as mqtt
import csv
from datetime import datetime

BROKER = "broker.hivemq.com"
TOPIC = "women_safety/heartrate"

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker")
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    hr = msg.payload.decode().strip()
    time_now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    print("Heart Rate received:", hr)

    with open("heart_rate_data.csv", "a", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([time_now, hr])

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, 1883, 60)
client.loop_forever()