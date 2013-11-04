#include <SoftwareSerial.h>

#define SEND_BYTE_TO_BYTE
#define CONTROL_CHECKSUM

// GPS PINS
#define SoftrxPin 2
#define SofttxPin 7
// SRF PINS
#define ECHOPIN1 3                            // Pin to receive echo pulse
#define TRIGPIN1 4                             // Pin to send trigger pulse
#define ECHOPIN2 5                            // Pin to receive echo pulse
#define TRIGPIN2 6                             // Pin to send trigger pulse

// initialisation de la liasion série
SoftwareSerial gps = SoftwareSerial(SoftrxPin, SofttxPin);
// variables
byte byteGPS = 0;
int i = 0;
int h = 0;
// Buffers for data input
char inBuffer[300] = "";
char GPS_RMC[100]="";
char GPS_GGA[100]="";

char *gga = "GGA";



void readExtractGpsGPGA(){
	// Read GGA sentence from GPS
	byteGPS = 0;
	byteGPS = gps.read();
	while(byteGPS != 'A'){
		byteGPS = gps.read();
	}
	GPS_GGA[0]='$';
	GPS_GGA[1]='G';
	GPS_GGA[2]='P';    
	GPS_GGA[3]='G';
	GPS_GGA[4]='G';
	GPS_GGA[5]='A';

	i = 6;
	while(byteGPS != '*'){                  
		byteGPS = gps.read();         
		inBuffer[i]=byteGPS;
		GPS_GGA[i]=byteGPS;
		i++;                      
	}

	// print the GGA sentence to USB
	//Serial.print("GPS GGA:");
	h = 0;
	while(GPS_GGA[h] != 42){
		//Serial.print(GPS_GGA[h],BYTE);
		Serial.print(GPS_GGA[h]);
		h++;
	}
	Serial.println();
}

void readExtractGpsGPRMC(){
	// Read the RMC sentence from GPS
	byteGPS = 0;
	byteGPS = gps.read();
	while(byteGPS != 'R'){
		byteGPS = gps.read();
	}
	GPS_RMC[0]='$';
	GPS_RMC[1]='G';
	GPS_RMC[2]='P';    
	GPS_RMC[3]='R';

	i = 4;
	while(byteGPS != '*'){                  
		byteGPS = gps.read();         
		inBuffer[i]=byteGPS;
		GPS_RMC[i]=byteGPS;
		i++;                      
	}  
	// print the RMC sentence to USB
	//Serial.print("GPS RMC:");
	h = 0;
	while(GPS_RMC[h] != 42){
		//Serial.print(GPS_RMC[h],BYTE);
		Serial.write(GPS_RMC[h]);
		h++;
	}
	Serial.println();
}


// DO NOTHING//////////////////////////////////////////////
float calculateDistance(int pinEcho, int pinTrig){
	digitalWrite(pinTrig, LOW);            // Set the trigger pin to low for 2uS
	delayMicroseconds(2);
	digitalWrite(pinTrig, HIGH);          // Send a 10uS high to trigger ranging
	delayMicroseconds(10);
	digitalWrite(pinTrig, LOW);           // Send pin low again
	int distance = pulseIn(pinEcho, HIGH); // Read in times pulse
	return distance/58.0;                  // Calculate distance from time of pulse                    
}

char chartoval(char c){
	if((c >=48) && (c<=57)){
		//it's a digit (0-9)
		return c-48;
	}else if((c>=65)&&(c<=70)){
		//it's a letter A-F
		return c-65+10;
	}else{
		return -1;
	}
}

char checksum(char *s) {
	char c = 0;

	s++;
	while(*s!='*')
		c ^= *s++;

	if(c!=chartoval(*(++s))*16+chartoval(*(++s))){
		c=0;
	}else{
		c=1;
	}

	return c;
}


void setup(){
	Serial.begin(9600);// ouvre le port série et règle le debit à 9600 bps
	gps.begin(9600);	// pareil pour les ports digitaux
	pinMode(SoftrxPin, INPUT);
	pinMode(ECHOPIN1, INPUT);
	pinMode(TRIGPIN1, OUTPUT);
	pinMode(ECHOPIN2, INPUT);
	pinMode(TRIGPIN2, OUTPUT);
}
int cpt=0;
char decompteOk;
void loop(){
	char str_buff[256];
	char buff;
	static int i=0;

	/*
		 if(cpt%4==3 ){
		 Serial.println("gprmc...");
		 readExtractGpsGPRMC();
		 Serial.println("gpga...");
		 readExtractGpsGPGA();
		 Serial.println("end gps...");
		 }
		 */
	if(gps.available()!=0){
#ifdef SEND_BYTE_TO_BYTE
		str_buff[i]=gps.read();
		if((str_buff[i++]=='\n')||(i>=255)){
			str_buff[i]=0;
			decompteOk=3;
			for(i=0;i<3;i++){
				if(str_buff[i+3]==gga[i]){
					decompteOk--;
				}
			}

			if(decompteOk<=0){
#ifdef CONTROL_CHECKSUM
				if(checksum(str_buff)==0){
					Serial.println("ERRORCHECKSUM");
				}else
#endif
				Serial.println(str_buff);
			}
			i=0;
		}
#else
		buff=gps.read();
		Serial.write(buff);
#endif
	}

	//Serial.print("$SRFR,");
	//Serial.println(calculateDistance(ECHOPIN1,TRIGPIN1));
	//Serial.print("$SRFL,");
	//Serial.println(calculateDistance(ECHOPIN2,TRIGPIN2));
	cpt++;
}

