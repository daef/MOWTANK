#define USBSERIAL 9600
#define BTGPSRATE 38400

#define SPDA             5          //Speed Control
#define DIR1A            9          //Direction
#define DIR2A            8          //Direction
#define SPDB             3          //Speed Control
#define DIR1B           12          //Direction
#define DIR2B           11          //Direction
#define STANDBY         10          //Standby Pin
#define STATUS_LED      13          //Led Pin
#define HAXXXOR          6          //spinning wheel of death 

#define MOTA             0
#define MOTB             3

#define FORWARD          1
#define BACKWARD         2
#define STOP             3

#define CMD_DRIVE       'c'
#define CMD_STOP        's'

#include <SoftwareServo.h> 


char MOTs[] = {SPDA, DIR1A, DIR2A, SPDB, DIR1B, DIR2B};
char x,y,z;
char status = CMD_STOP;
SoftwareServo myservo;  // create servo object to control a servo 

char readBT() { while(!Serial1.available()) sdelay(20); return (char)Serial1.read(); }

void sendBT(char command[]) {
    Serial1.println();
    sdelay(250);
    Serial1.print(command);
    sdelay(250);
    Serial1.println();
    Serial1.flush();
    sdelay(250);
}

void setupBT(int setupdelay) {
    sendBT("+STWMOD=0");            // client mode
    sendBT("+STNA=m0wb0t");         // device name
    sendBT("+STAUTO=0");            // auto connect forbidden
    sendBT("+STOAUT=1");            // permit paired device to connect me
    sendBT("+STPIN=1337");          // bluetooth pin 
    delay(setupdelay);              // This delay is required.
    sendBT("+INQ=1");               // enable inqure the device 
    delay(setupdelay);              // This delay is required.
}

void stopMotor(char mot) { setMotor(mot, STOP, 0); }

void setMotor(char mot, char dir, int spd) {
    analogWrite (MOTs[mot + 0], spd);
    digitalWrite(MOTs[mot + 1], dir & 0x01);
    digitalWrite(MOTs[mot + 2], dir & 0x02);
    myservo.write(spd/2);
}

void fullStop() {
    digitalWrite(STANDBY,    LOW);
    digitalWrite(STATUS_LED, LOW); // indicate status
    stopMotor(MOTA);
    stopMotor(MOTB);
    myservo.write(0);
    SoftwareServo::refresh();
    status = CMD_STOP;
}

void sdelay(int servodelay)
  {
    unsigned long previousMillis = 0; // last time update
    int sloop = 0;
    for (sloop = 0; sloop < servodelay; sloop +=1)
      {
        unsigned long currentMillis = millis();  
          if(currentMillis - previousMillis > 40) {  
           previousMillis = currentMillis;  
           SoftwareServo::refresh();  // keep the softservo pbm alive 
           }
        delay(1);
      }
  }
  
void initservo() {
    int pos = 0;   
    for(pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    sdelay(20);
  } 
  sdelay(100);
  for(pos = 180; pos>=1; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    sdelay(20);
    SoftwareServo::refresh();
  } 
  
}

void setup() {
    pinMode( SPDA       , OUTPUT );
    pinMode( SPDB       , OUTPUT );
    pinMode( STANDBY    , OUTPUT );
    pinMode( STATUS_LED , OUTPUT );
    pinMode( DIR1A      , OUTPUT );
    pinMode( DIR2A      , OUTPUT );
    pinMode( DIR1B      , OUTPUT );
    pinMode( DIR2B      , OUTPUT );
                         
    myservo.attach(HAXXXOR);  // attaches the servo pin
    initservo();
    
    fullStop(); //init's all previous outputs
    
    Serial.begin(USBSERIAL);        // init USB Serial (console)
    Serial1.begin(BTGPSRATE);       // init pin0/1 Serial (bluetooth)
    delay(1234); 
    setupBT(3210);
    while(readBT() != CMD_STOP);    // safety frist? ¯\(°_o)/¯ I DUNNO LOL
}

int runDelay = 0;
void loop() {
    Serial1.print('r');             // request data from droid
    char bt = readBT();
    if(bt == CMD_STOP)
        fullStop();
    else if(bt == CMD_DRIVE) {
        if(status == CMD_STOP) {
            status = CMD_DRIVE;
            digitalWrite(STANDBY,    HIGH);
            digitalWrite(STATUS_LED, HIGH); // indicate status
        }
        x = readBT(); y = readBT(); z = readBT();
        int l = z + y, r = z - y;
        int L = (l < 0 ? -l : l)<<1;
        int R = (r < 0 ? -r : r)<<1;
        L = L > 255 ? 255 : L < 0 ? 0 : L;
        R = R > 255 ? 255 : R < 0 ? 0 : R;
        
        setMotor(MOTA, -l > 0 ? FORWARD : BACKWARD, L);
        setMotor(MOTB, -r > 0 ? FORWARD : BACKWARD, R);
    }
}
