#include <WiFiNINA.h>
#include <PubSubClient.h>


const char* ssid = "DODO-166E";//This is the name of my wifi
const char* password = "EJTZG2WUNN";//This is the password of my wifi


const char* mqttServer = "broker.emqx.io";//We are using this as the MQTT server
const int mqttPort = 1883;//We are using 1883 as the port
const char* mqttTopic = "SIT210/wave";
const char* mqttPatTopic = "SIT210/pat";  // This is a topic we will sunscribe for pat action.

// Define Hardware Pins
#define TRIG_PIN 2 //This define the positions of pins on iur hardware setup
#define ECHO_PIN 3
#define LED_PIN  4

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Function to Connect to WiFi
void connectWiFi() {// this makes sure that the wifi ets connected to our setup
  Serial.print("Connecting to WiFi...");//It is printed till the time wifi is not connedted
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");//This prints as soon as the wifi is connected
}


void connectMQTT() {//This is used to cnnect to our setup to the MQTT broker
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");//It is first printed before finally connecting to MQTT broker
    String clientID = "Arduino_" + String(random(1000)); //The client ID is a unique id that is automatically generated or you can also make one
    if (client.connect(clientID.c_str())) {
      Serial.println("Connected!");
      if (client.subscribe(mqttTopic) && client.subscribe(mqttPatTopic)) {  // Subscribe to both topics
        Serial.println("Subscribed to topics: " + String(mqttTopic) + ", " + String(mqttPatTopic));//Now after connection we need to subscribe to our topic as well, so it gets subscribed to our topic
      } else {
        Serial.println("Failed to subscribe!");
      }
    } else {
      Serial.print("Failed. Error Code: ");//This happens when it is not able to read the code.
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// Callback Function for Incoming Messages
void callback(char* topic, byte* payload, unsigned int length) {/* thsi function gets triggered automatically as soon as an MQTT message is received from the broker.
  It is basically used for reading the message and printing it out to the serial monitor.*/
  Serial.print("Message received on topic ");
  Serial.print(topic);
  Serial.print(": ");

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Check message and blink LED differently
  if (String(topic) == mqttTopic) {  // "wave" handler
    Serial.println("Blinking LED for wave...");
    for (int i = 0; i < 3; i++) {  // Blink 3 times (500ms ON, 500ms OFF)
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    }
  } else if (String(topic) == mqttPatTopic) {  // This one code is introduced for led to behave differently for pat
    Serial.println("Blinking LED for pat..."); //for the pat it will blink differently
    for (int i = 0; i < 5; i++) {  // Blink 5 times for the pat
      digitalWrite(LED_PIN, HIGH);//Makes the LED glow
      delay(200);
      digitalWrite(LED_PIN, LOW); //Turns off the LED
      delay(200);
    }
  }
}


float getDistance() {//THis function us set to measure the distance by using our ultrasonic sensor.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  float duration = pulseIn(ECHO_PIN, HIGH);
  float distance = (duration * 0.0343) / 2;  // Convert to cm
  return distance;
}

void setup() {
  Serial.begin(115200);//It is used for starting the serial communication with the serial monitor.
  pinMode(TRIG_PIN, OUTPUT);//It is used for setting uup the trig pin for output.
  pinMode(ECHO_PIN, INPUT);//It is used for setting up the echo pin as the inout 
  pinMode(LED_PIN, OUTPUT);//Sets up the led pins for gving the output

  connectWiFi();//For making a connection to the wifi
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  connectMQTT();//Used for connecting to the mqtt BROKER WHICH WE ARE USING AS broker.emqx.io
}

void loop() {
  if (!client.connected()) {
    connectMQTT();//It is used for connecting to MQTT again if its discinnected. 
  }
  client.loop();

  float distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 35) { // If motion is detected within 35 cm, then it will give the follwoing messages oout on the serial monitor
    Serial.println("Motion detected! Publishing 'wave' message...");
    if (client.publish(mqttTopic, "Vasu waved!")) {//This message appears to us as soon as the disatnce becomes less than the threshold value.
      Serial.println("Message sent successfully!");
    } else {
      Serial.println("Failed to send message.");
    }
    delay(2000);  
  } else if (distance >= 35 && distance < 50) {  // If motion is detected greater 35 cm and less than 50, then it will give the follwoing messages oout on the serial monitor
    Serial.println("Motion detected! Publishing 'pat' message...");///The following will appear on the serial monitor
    if (client.publish(mqttPatTopic, "Vasu patted!")) {//This message appears to us on the channnel that is Vasu patted!
      Serial.println("Message sent successfully!");
    } else {
      Serial.println("Failed to send message.");
    }
    delay(2000);
  }

  delay(1000);  // Read distance every 1 second
}
