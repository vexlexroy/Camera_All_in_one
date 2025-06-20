const int LED_PIN = 13;   // Set pin 13 as LED pin
String incomingMessage = "";

void setup() {
  Serial.begin(9600);       
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output
}

void loop() {
  if (Serial.available() > 0) {      
    incomingMessage = Serial.readStringUntil('\n');
    incomingMessage.trim(); 

    if (incomingMessage == "LEDTESTON") {
      digitalWrite(LED_PIN, HIGH);    // Turn LED ON 
    }

    if (incomingMessage == "LEDTESTOFF") {                
      digitalWrite(LED_PIN, LOW);     // Turn LED OFF
    }

    if(incomingMessage == "TEST"){
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LEDON\n");
    }
  }
}

