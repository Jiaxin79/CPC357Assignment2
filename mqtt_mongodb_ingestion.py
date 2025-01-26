import pymongo
import paho.mqtt.client as mqtt
import json
from datetime import datetime, timezone
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi

# MongoDB Configuration
uri = "mongodb+srv://<username>:<password>@<clustername>.jbbt9.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0"

# Create a new MongoDB client and connect to the server
try:
    mongo_client = MongoClient(uri, server_api=ServerApi('1'))
    mongo_client.admin.command('ping')
    print("Pinged your deployment. You successfully connected to MongoDB!")
    
    # Define the database and collection
    db = mongo_client["smarthome"]  # Replace with your database name 
    collection = db["iot"]  # Replace with your collection name 
except Exception as e:
    print(f"Error connecting to MongoDB: {e}")
    exit(1)

# MQTT Configuration
mqtt_broker_address = "34.59.5.29"
mqtt_topic = "iot"

# Define the callback function for MQTT connection
def on_connect(mqtt_client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Successfully connected to MQTT broker")
        mqtt_client.subscribe(mqtt_topic)
    else:
        print(f"Failed to connect, return code {reason_code}")

# Define the callback function for processing incoming MQTT messages
def on_message(mqtt_client, userdata, message):
    try:
        payload = message.payload.decode("utf-8")
        print(f"Received message: {payload}")

        # Parse JSON payload
        data = json.loads(payload)

        # Add timestamp
        timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        document = {"timestamp": timestamp, "data": data}

        # Insert into MongoDB
        collection.insert_one(document)
        print("Data ingested into MongoDB successfully")

    except json.JSONDecodeError:
        print("Error: Received payload is not valid JSON")
    except Exception as e:
        print(f"Error processing message: {e}")
# Create an MQTT client instance
mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# Attach the callbacks
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Connect to the MQTT broker
try:
    mqtt_client.connect(mqtt_broker_address, 1883, 60)
    print("Connecting to MQTT broker...")
except Exception as e:
    print(f"Error connecting to MQTT broker: {e}")
    exit(1)

# Start the MQTT loop to listen for messages
mqtt_client.loop_forever()
