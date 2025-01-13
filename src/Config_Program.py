import paho.mqtt.client as mqtt
import requests
import json
import datetime

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

def on_message(client, userdata, message):
    # userdata is the structure we choose to provide, here it's a list()
    userdata.append(message.payload)
    # We only want to process 10 messages

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print(f"Failed to connect: {reason_code}. loop_forever() will retry connection")
    else:
        # we should always subscribe from on_connect callback to be sure
        # our subscribed is persisted across reconnections.
        client.subscribe("config/system")

def on_message(client, userdata, msg):
    print(msg.topic+" "+msg.payload.decode('utf-8'))
    topic_string = msg.topic.split("/")
    subject = topic_string[0]
    action = topic_string[1]
    print(subject + " " + action + " checking splitting")

def fetch_data(latitude, longitude, tzid):
    url = "https://api.sunrise-sunset.org/json"
    params = {
        "lat": latitude,
        "lng": longitude,
        "formatted": 0,
        "date": "today",
        "tzid": tzid
    }
    try:
        # Efectuarea cererii HTTP GET
        response = requests.get(url, params=params)

        # Verificarea statusului răspunsului
        if response.status_code == 200:
            # Parsarea răspunsului JSON
            data = response.json()
            
            # Extrage datele utile
            sunrise = data['results']['sunrise']
            sunset = data['results']['sunset']
            
            # Afișează informațiile
            #print(f"Sunrise: {sunrise}")
            #print(f"Sunset: {sunset}")
            sunrise_time = datetime.datetime.fromisoformat(sunrise)
            sunset_time = datetime.datetime.fromisoformat(sunset)
            return sunrise_time, sunset_time
        else:
            print("Eroare la obținerea datelor")
    except Exception as e:
        print(f"An error occurred: {e}")

def sunset_sunrise_calc(delta_sunset, delta_sunrise):
    latitude = input("Enter the latitude: ")
    longitude = input("Enter the longitude: ")
    if latitude == "" or longitude == "":
        print("Invalid input")
        return

    tzid = input("Enter timezone format:(e.g. Europe/Bucharest): ")
    print("Sunset/Sunrise calculation")
    sunrise_hour, sunset_hour = fetch_data(latitude, longitude, tzid)
    sunrise_hour = sunrise_hour + datetime.timedelta(minutes=int(delta_sunrise))
    sunset_hour = sunset_hour + datetime.timedelta(minutes=int(delta_sunset))
    print(f"Deactivation hour: {sunrise_hour}")
    print(f"Activation hour: {sunset_hour}")
    json_data = {
        "Activation": sunset_hour.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "Deactivation": sunrise_hour.strftime("%Y-%m-%dT%H:%M:%S%z")
    }
    return json_data

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe
mqttc.on_message = on_message

mqttc.user_data_set([])
mqttc.connect("192.168.0.199", 1883)

unacked_publish = set()
mqttc.loop_start()
while True:
    print("Config types: ")
    print("1. Sunset/Sunrise")
    print("2. Specific time")
    print("3. Exit")

    message = input("Enter the number of the configuration you want to set: ")
    if message == "1":
        delta_sunset = input("How many minutes after sunset should the lights turn on? ")
        delta_sunrise = input("How many minutes before sunrise should the lights turn off? ") 
        config_message = sunset_sunrise_calc(delta_sunset, delta_sunrise)
        msg_info = mqttc.publish("config/system", json.dumps(config_message), qos=1)
        msg_info.wait_for_publish()
        print("/n")
    elif message == "2":
        activation_time = input("Enter the activation time(format %Y-%m-%dT%H:%M:%S%z): ")
        deactivation_time = input("Enter the deactivation time(format %Y-%m-%dT%H:%M:%S%z): ")
        config_message = {
            "Activation": activation_time,
            "Deactivation": deactivation_time
        }
        msg_info = mqttc.publish("config/system", json.dumps(config_message), qos=1)
        msg_info.wait_for_publish()
        print("/n")
    elif message == "3":
        print("Exiting the program")
        break
    else:
        print("Invalid input")
        print("/n")
        continue
    #msg_info = mqttc.publish("config/system", message, qos=1)
    #msg_info.wait_for_publish()
    #print(f"Received the following message: {mqttc.user_data_get()}")

mqttc.loop_stop()