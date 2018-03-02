/*
 Based on Reconnecting MQTT example - non-blocking

 This sketch demonstrates how to keep the client connected
 using a non-blocking reconnect function. If the client loses
 its connection, it attempts to reconnect every 5 seconds
 without blocking the main loop.

 Stor 10 messages MQTT for display in loop on LCD screen 2 lines of 16 char

 Exemple to send MQTT with client mosquitto
 mosquitto_pub -h 192.168.0.66 -t "maison/aff/2x16A" -m  "{\"L1\":\"Contenu Ligne 1\",\"L2\":\"Contenu ligne 2\"}"
 
*/

#define DEBUG
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>

// Pin use by program
LiquidCrystal lcd(0, 2, 4, 14, 12, 13);

// Update these with values suitable for your hardware/network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 38);                          // Adress Wemos or D1 ou Arduino Ethernet on network
IPAddress server(192, 168, 0, 66);                      // Server MQTT

const char* ssid = "SSID";                              // Your WIFI SSID 
const char* password = "PASSWORD";                      // Your WIFI password

// Le topics a écouter pour recevoir des ordres
#define SUBSCRIBELOG         "maison/aff/2x16"           //Topic qui permettra de recevoir des ordres a executer 
#define SUBSCRIBEAUT         "maison/aff/2x16A"          //Topic automatique de Jeedom 

int iInfoLog = 0;             // Si c'est à 0 pour automatique on affiche pas les log si c'est à 1 on affiche que les LOG

// Variable pour memorisation messages
int numMsgsInArray = 0;         // This has a maximum of 10 messages
String messagesL1[10];          // because we've defined 10 Object Strings first line and key
String messagesL2[10];          // because we've defined 10 Object Strings seconde line
int msg_num = 1;                // This is between 1 and 10, reflecting which message number is printed on the LCD
int msg_index = 99;             // Index for storage message
int msg_index_current = 0;      // Index message on screen
long lastPressKey   = 0;        // Tempo pour bloquer defilement automatique si touche utilisée
long lastAffichage  = 0;        // Pour compteur tempo d'affichage sans utiliser delay 
long tempoAffichage = 1000;     // Duree tempo d'affichage d'un message 


/* 
 * Search if message MQTT is already stored 
 */
boolean searchMessage(char *newMsgSearch){
    boolean find = false;
    #ifdef DEBUG
      Serial.println("Recherche de :");
      Serial.println(newMsgSearch);
    #endif
    // On cherche dans le tableau des messages
    for (int i = 1; i <= numMsgsInArray; i++){
      Serial.println(messagesL1[i-1]);
      if (messagesL1[i-1] == newMsgSearch){
        #ifdef DEBUG
          Serial.println("Message déjà existant");
        #endif
        msg_index = i-1;
        find = true;
      }
    }
    return find;    
}

/*
 * Add message in array
 */

void addMessage(char *newMsg, char *newMsg2){
   #ifdef DEBUG
    Serial.println("Ajout du message :");
    Serial.print(newMsg);
   #endif   
   numMsgsInArray++;
   if (numMsgsInArray >= 11){ 
       // To shift the messages up in the array,
       // get the length of the string and reserve it,
       // Then shift the msg to the reserved space.
       // Note that I have used "human values" so that "number of messages" is correct (between 1 and 10), and
       // the code subtracts 1, to point to the correct array entry (between 0 and 9).
       for (int i = 1; i <= 9; i++){
          int tmp1Length = (messagesL1[i].length());
          messagesL1[i-1].reserve(tmp1Length);
          messagesL1[i-1] = messagesL1[i];
       }
     numMsgsInArray = 10;
     }

  // Premier tableau messagesL1
  int strLength = strlen(newMsg)+1; //Include space for \0
  messagesL1[numMsgsInArray-1].reserve(strLength);
  messagesL1[numMsgsInArray-1] = newMsg;

  // Deuxieme tableau messagesL2
  int strLength2 = strlen(newMsg2)+1; //Include space for \0
  // Je vais forcer la taille memoire a 17 caractere pour les 16 caractere de la ligne + \0
  // Pour ligne 1 c'était different car c'est une clef donc il faut utiliser tel que 
  strLength2 = 17;
  //messagesL2[numMsgsInArray-1].reserve(strLength2);
  messagesL2[numMsgsInArray-1] = newMsg2;
  
}

/*
 * Print message on screen
 */
void defilMessages(){
  // Si pas de message pas de defilement
  if ( numMsgsInArray == 0 ) return;
  // Si touche recement pressee pas de defilement on obeit a l'affichage touche
  long nowPress = millis();
  if (nowPress - lastPressKey < 2000) {      
    return;
  }
  // Ok on ne change pas d'affiche pendant toute la duree de tempoAffichage
  long nowAffichage = millis();
  if (nowAffichage - lastAffichage > tempoAffichage) {
    lastAffichage = nowAffichage;
    // Ok maintenant on fait defiler
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(messagesL1[msg_index_current]);
    lcd.setCursor(0, 1);
    lcd.print(messagesL2[msg_index_current]);
    msg_index_current++;
    if (msg_index_current >= numMsgsInArray){
        msg_index_current = 0;
    }
  }
}

/*
 * Print message when key pressed
 */
void printMessages() {
   #ifdef DEBUG
    // If you want to see the messages on your serial monitor (eg for testing the code)
    Serial.println("List stored messages");
    for (int i = 1; i <= numMsgsInArray; i++){
      Serial.println(messagesL1[i-1]);
    }
  #endif
  //
  // Now, print the messages out to the LCD screen.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(messagesL1[msg_num]);
  lcd.setCursor(0, 1);
  lcd.print(messagesL2[msg_num]);  
}


void checkUpButton() {
  msg_num--;
  if (msg_num < 1) {
    msg_num = 0;
  }
  #ifdef DEBUG
    Serial.print("checkUpButton  :");
    Serial.println(msg_num);
    Serial.print("Le msg est  :");
    Serial.println(messagesL1[msg_num]);
  #endif               
  printMessages();
  lastPressKey = millis();  
}

void checkDownButton() {
  msg_num++;
  if (msg_num >= numMsgsInArray) {
    msg_num = numMsgsInArray-1;
  }
  #ifdef DEBUG
    Serial.print("checkDownButton  :");
    Serial.println(msg_num);
    Serial.print("Le msg est  :");
    Serial.println(messagesL1[msg_num]);  
  #endif
  printMessages();
  lastPressKey = millis();
}


/*
 * Detect key pressed
 */
void pressButton() {
  int x;
  x = analogRead (0);
  //Serial.println(x,DEC);
  if (x < 100) {               // Left=769  Right=9 Up=225 Down=500
     #ifdef DEBUG
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print ("Right");
      Serial.print("The value at pin A0 Right key pressed is  :");
      Serial.println(x,DEC);
     #endif

     checkUpButton();
     
  } else  if (x < 250) {
            #ifdef DEBUG
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print ("Up    ");
              Serial.print("The value at pim A0 UP key pressed is  :");
              Serial.println(x,DEC);
            #endif
        
            checkUpButton();

          } else  if (x < 550){
                    #ifdef DEBUG
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print ("Down  ");
                      Serial.print("The value at pim A0 Down key pressed is  :");
                      Serial.println(x,DEC);
                    #endif
                    
                    checkDownButton();
               
                  } else  if (x < 800){
                            #ifdef DEBUG
                              lcd.clear();
                              lcd.setCursor(0, 0);
                              lcd.print ("Left  ");
                              Serial.print("The value at pim A0 Left key pressed is  :");
                              Serial.println(x,DEC);
                            #endif

                            checkDownButton();
                            
                          } //else if (x < 800){
                        //        lcd.clear();
                        //        lcd.setCursor(0, 0);  
                        //        lcd.print ("Select");
                        //        Serial.print("The value at pim A0 Select key pressed is  :");
                        //        Serial.println(x,DEC);
                        //      }
  /*
   * Imperatif laisser delay sinon bouclage sur Attempt to reconnect
   * Surement une histoire de analogRead qui interfere ????????
   */
  delay(200);   // Give the user time to lift their finger off the button
}

/*
 * Traitment MQTT message
 */
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  #ifdef DEBUG
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  
    for (int i=0;i<length;i++) {
      Serial.print((char)payload[i]);  
    }
    Serial.println();
  #endif
  StaticJsonBuffer<200> jsonBuffer;
  char inData[80];
  for(int i = 0; i<length; i++){
    // Serial.print((char)payload[i]);
    inData[i] = (char)payload[i];
  }
  
  JsonObject& root = jsonBuffer.parseObject(inData);
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
    
  // En fonction du mode on affiche seulement ce qui vient de jeedom (mode A) ou alors tous les logs (mode L)
  const char* L1    = root["L1"];
  // J'ai trouvé cette bidouille pour contourner le probleme de const char de la lib JSON
  String ligne1 = root["L1"];
  char char_arr[ligne1.length()];
  ligne1.toCharArray(char_arr, ligne1.length()+1);
  
 //const char* L1    = root["L1"];
  const char* L2    = root["L2"];
  String ligne2 = root["L2"];
  char char_arr2[ligne2.length()];
  ligne2.toCharArray(char_arr2, ligne2.length()+1);


  // Si le message n'existe pas il faut l'ajouter
  if (!searchMessage(char_arr)){
      addMessage(char_arr,char_arr2);
  } else {
      // On efface la ligne
      messagesL2[msg_index] = "                ";
      // On affecte la ligne
      messagesL2[msg_index] = char_arr2;
  }  
}

WiFiClient ethClient;
PubSubClient client(ethClient);

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("arduiCli")) {
    // Once connected, publish an announcement...
    client.publish("outTopic","hello world arduino messagerie");
    // ... and resubscribe
    // client.subscribe("inTopic");
    if ( iInfoLog == 0) {
      client.subscribe(SUBSCRIBEAUT);
      client.unsubscribe(SUBSCRIBELOG);
    } else {
      client.subscribe(SUBSCRIBELOG);
      client.unsubscribe(SUBSCRIBEAUT);
    }
  }
  return client.connected();
}

void setup() {
  // For test without MQTT messages Creation de 3 messages artificiellement
  //addMessage("1 first  Hello","220V");
  //addMessage("2 second World","18 C");
  //addMessage("3 third  msg  ","1400W");
  
  Serial.begin(115200);
  client.setServer(server, 1883);
  client.setCallback(callback);
  lcd.begin(16, 2);  
  lcd.setCursor(0, 0);
  lcd.print("Try Con. ");
  lcd.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  IPAddress myIp = WiFi.localIP();
  #ifdef DEBUG
      Serial.println("");
      Serial.print("Connect ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
  #endif      
  //Ethernet.begin(mac, ip);
  lcd.setCursor(0, 0);
  lcd.print(ssid);
  lcd.print(" <=> ok");
    
  // On affiche sur le LCD à la lgine 1 l'adresse IP
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(1500);
  lastReconnectAttempt = 0;
}


void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      #ifdef DEBUG
        Serial.println("Attempt to reconnect");
      #endif
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    // Ici le code à executer
    pressButton();
    defilMessages();
    client.loop();
  }

}
