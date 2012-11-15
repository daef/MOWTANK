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

#define MOTA             0
#define MOTB             3

#define FORWARD          1
#define BACKWARD         2
#define STOP             3

#define CMD_DRIVE 'c'
#define CMD_STOP 's'


/*
s => stop
cBBB => drive x y z

*/

char MOTs[] = {SPDA, DIR1A, DIR2A, SPDB, DIR1B, DIR2B};
char x,y,z;
char status = CMD_STOP;

char readBT() { while(!Serial1.available()) delay(23); return (char)Serial1.read(); }

void sendBT(char command[]) {
    Serial1.println(command);
    Serial1.flush();
}

void setupBT(int setupdelay) {
    sendBT("daef was here");
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
}

void fullStop() {
    digitalWrite(STANDBY,    LOW);
    digitalWrite(STATUS_LED, LOW); // indicate status
    stopMotor(MOTA);
    stopMotor(MOTB);
    status = CMD_STOP;
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
    fullStop();                     //init's all previous outputs

    Serial.begin(USBSERIAL);        // init USB Serial (console)
    Serial1.begin(BTGPSRATE);       // init pin0/1 Serial (bluetooth)
    delay(1234); 
    setupBT(2345);
    while(readBT() != CMD_STOP);    // safety frist? ¯\(°_o)/¯ I DUNNO LOL
}

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
        setMotor(MOTA, y, FORWARD);
        setMotor(MOTB, z, FORWARD);
    }
}
