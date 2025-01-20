import paho.mqtt.client as mqtt
import json
from datetime import datetime
#from tabulate import tabulate
import pandas as pd
import ssl, time, inspect, os
database = pd.read_csv("database.csv")

def on_subscribe(client, userdata, mid, reason_code_list, properties):
    # Since we subscribed only for a single channel, reason_code_list contains
    # a single entry
    if reason_code_list[0].is_failure:
        print(f"Broker rejected you subscription: {reason_code_list[0]}")
    else:
        print(f"Broker granted the following QoS: {reason_code_list[0].value}")

def on_unsubscribe(client, userdata, mid, reason_code_list, properties):
    # Be careful, the reason_code_list is only present in MQTTv5.
    # In MQTTv3 it will always be empty
    if len(reason_code_list) == 0 or not reason_code_list[0].is_failure:
        print("unsubscribe succeeded (if SUBACK is received in MQTTv3 it success)")
    else:
        print(f"Broker replied with failure: {reason_code_list[0]}")
    client.disconnect()


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print(f"Failed to connect: {reason_code}. loop_forever() will retry connection")
    else:
        # we should always subscribe from on_connect callback to be sure
        # our subscribed is persisted across reconnections.
        client.subscribe("info/activity")

def on_message(client, userdata, msg):
    payload = msg.payload.decode('utf-8')
    json_payload = json.loads(payload)

    if "Time" not in json_payload or "Senzor" not in json_payload:
        print("Invalid message")
        return
    
    time_str = json_payload["Time"]
    sensor = json_payload["Senzor"]
    
    time_obj = datetime.strptime(time_str, "%Y-%m-%dT%H:%M:%S")
    day = time_obj.strftime("%Y-%m-%d")
    time = time_obj.strftime("%H:%M:%S")
    dictionary = {
        "Day": day,
        "Time": time,
        "Sensor": sensor
    }
    df_dict = pd.DataFrame([dictionary])
    global database
    database = pd.concat([database, df_dict], ignore_index=True)

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, "Database")
mqttc.tls_set(ca_certs="ca.crt", tls_version=ssl.PROTOCOL_TLSv1_2, ciphers=None)
mqttc.tls_insecure_set(True)
mqttc.username_pw_set("proiect", "1234")
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe


mqttc.user_data_set([])
mqttc.connect("192.168.43.212", 8883)

unacked_publish = set()
mqttc.loop_start()
while True:
    print("Available commands:")
    print("1. Show")
    print("2. Show data from a specific day")
    print("3. Show data from a specific sensor")
    print("4. Show data from a specific sensor from a specific day")
    print("5. Exit")
    command = input("Enter the number of a command: ")
    if command == "5":
        database.to_csv("database.csv", index=False)
        print("Database was saved in database.csv")
        break
    elif command == "1":
        print('\n')        
        print(database)
        print("\n")
    elif command == "2":
        day = input("Enter the day(YYYY-MM-DD): ")
        print('\n')
        print(database[database["Day"] == day])
        print("\n")
    elif command == "3":
        sensor = input("Enter the sensor: ")
        print('\n')
        print(database[database["Sensor"] == sensor])
        print("\n")
    elif command == "4":
        sensor = input("Enter the sensor: ")
        day = input("Enter the day(YYYY-MM-DD): ")
        print('\n')
        print(database[(database["Sensor"] == sensor) & (database["Day"] == day)])
        print("\n")
    else:
        print("Invalid command")
        print("\n")

mqttc.loop_stop()