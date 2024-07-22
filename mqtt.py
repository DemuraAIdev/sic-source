import paho.mqtt.client as mqtt
import pickle
import numpy as np
import json
from langchain_openai import ChatOpenAI
from dotenv import load_dotenv
import os

load_dotenv()
OPENAI_API_KEY = os.getenv('API_KEY')

os.environ['OPENAI_API_KEY'] = OPENAI_API_KEY



llm = ChatOpenAI(model="gpt-3.5-turbo-1106", temperature=0)

all_data = []

def load_model(filename):
    # Load the model from the file
    with open(filename, 'rb') as file:
        model = pickle.load(file)
    return model



def preprocess_temperature(data, scaler):
    data = np.array(data)
    data = data[:,0]
    data = scaler.transform([data])
    return data

def preprocess_humidity(data, scaler):
    data = np.array(data)
    data = data[:,1]
    data = scaler.transform([data])
    return data

def preprocess_aqi(data, scaler, poly):
    data = np.array(data)
    data = data[:,2]
    data = scaler.transform([data])
    data = data.reshape(1, -1)
    data = poly.transform(data)
    return data

def dump_data(temperature, humidity, aqi):
    global all_data
    all_data.append([temperature, humidity, aqi])
    all_data = all_data[-200:]
    return all_data

temp_scaler = load_model('temp_scaler.pkl')
temp_model = load_model('temp_model.pkl')
humid_scaler = load_model('hum_scaler.pkl')
humid_model = load_model('hum_model.pkl')
aqi_scaler = load_model('AQ_scaler.pkl')
aqi_model = load_model('AQ_model.pkl')
aqi_poly = load_model('AQ_poly.pkl')


def on_subscribe(client,userdata, mid, reason_code_list, properties):
    # Since we subscribed only for a single channel, reason_code_list contains
    # a single entry
    if reason_code_list[0].is_failure:
        print(f"Broker rejected you subscription: {reason_code_list[0]}")
    else:
        print(f"Broker granted the following QoS: {reason_code_list[0].value}")

def on_unsubscribe(client,usersata, mid, reason_code_list, properties):
    # Be careful, the reason_code_list is only present in MQTTv5.
    # In MQTTv3 it will always be empty
    if len(reason_code_list) == 0 or not reason_code_list[0].is_failure:
        print("unsubscribe succeeded (if SUBACK is received in MQTTv3 it success)")
    else:
        print(f"Broker replied with failure: {reason_code_list[0]}")
    client.disconnect()


def on_message(client,userdata, message):
    print(f"Received message from topic {message.topic}: {message.payload.decode('utf-8')}")
    if message.topic == "esp32/sensor/train":
        data = json.loads(message.payload.decode('utf-8'))
        temperature = data["temperature"]
        humidity = data["humidity"]
        aqi = data["mq135"]  # Assuming mq135 is the AQI
        
        input_prompt = f"""
        Berikut adalah data dari sensor ESP32
        temperature: {temperature} 
        humidity: {humidity} 
        AirQuality: {aqi} 

        Berikan Saran agar ruangan menjadi sehat :"""

        result = llm.invoke(input_prompt)
        if len(all_data) == 200:

            dump_data(temperature, humidity, aqi)

            # Temperature inference
            temp_data = preprocess_temperature(all_data, temp_scaler)
            temp_pred = temp_model.predict(temp_data)
            temp_pred = temp_pred[0][0]
            print(f"Temperature prediction: {temp_pred}")
            # Humidity inference
            humid_data = preprocess_humidity(all_data, humid_scaler)
            hum_pred = humid_model.predict(humid_data)
            hum_pred = hum_pred[0][0]
            print(f"Humidity prediction: {hum_pred}")
            # AQI inference
            aqi_data = preprocess_aqi(all_data, aqi_scaler, aqi_poly)
            aqi_pred = aqi_model.predict(aqi_data)
            aqi_pred = aqi_pred[0]
            print(f"AQI prediction: {aqi_pred}")
            client.publish("esp32/sensor/prediction", json.dumps({"temperature": temperature, "humidity": humidity, "aqi": aqi, "prediction": {"temperature": temp_pred, "humidity": hum_pred, "aqi": aqi_pred}}))

            client.publish("esp32/sensor/response", result.content) 
        else:
            dump_data(temperature, humidity, aqi)
            client.publish("esp32/sensor/prediction", json.dumps({"temperature": temperature, "humidity": humidity, "aqi": aqi, "prediction": False}))

            # publish AI response
            client.publish("esp32/sensor/response", result) 
            print("Not enough data to make a prediction")
            # show current amount of data
            print(f"Current data: {len(all_data)}")
    # We only want to process 10 messages-

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print(f"Failed to connect: {reason_code}. loop_forever() will retry connection")
    else:
        # we should always subscribe from on_connect callback to be sure
        # our subscribed is persisted across reconnections.
        client.subscribe("esp32/sensor")
        client.subscribe("esp32/sensor/train")
        client.subscribe("esp32/sensor/response")

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# set tls
mqttc.tls_set()

username = "cyberai"  # Replace with your actual username
password = "Min2kota"  # Replace with your actual password
mqttc.username_pw_set(username, password)
mqttc.connect("0894e758ae594242b41480fb24e2f7de.s1.eu.hivemq.cloud", 8883, 60)

mqttc.loop_forever()
