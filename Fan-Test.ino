/*  ESP8266, DS18B20 with updates to ThinkSpeak.com
*
*  Supports multiple DS18B20 sensors on one pin, see Dallas datasheet for
*  wiring schematics.
*
*  Requires free ThingSpeak account: https://thingspeak.com/
*  Requires the following libraries to be installed:
*
*  OneWire.h - Built in.
*  ESP8266Wifi.h - Built in to ESP8266/Arduino integration.
*  DallasTemperature.h - Dallas Temperature sensor  library by Miles Burton:
*  https://github.com/milesburton/Arduino-Temperature-Control-Library
*  ThingSpeak.h - Offical ThinkSpeak library by Mathworks:
*  https://github.com/mathworks/thingspeak-arduino
*
*  Portions of code taken from Dallas Temperature libray examples by Miles Burton.
*
*  To test sensor readings without uploading to ThinkSpeak cooment out
*  line 144 (ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);)
*
*/

#include <DallasTemperature.h>
#include <OneWire.h>
#include <ThingSpeak.h> 
#include <ESP8266WiFi.h>

// User changeable vaules go here.

#define ONE_WIRE_BUS D4                           // Digital pin DS18B20 is connected to.
#define TEMPERATURE_PRECISION 12                  // Set sensor precision.  Valid options 8,10,11,12 Lower is faster but less precise

unsigned long myChannelNumber = 85519;            // Thingspeak channel ID here
const char * myWriteAPIKey = "";  // Write API key here
const char* ssid = "VM166468-2G";             // SSID of wireless network
const char* password = "";                // Password for wireless network

int fieldStart = 4;                               // Field number to start populating ThingSpeak channel data, leave at 
												  // 1 if this is the only device reporting to that channel.  
												  // If more than one device this should be the FIRST FREE field.

int updatePeriod = 180;                            //delay in seconds to update to ThingSpeak.  Should be set to not less than 15.


int status = WL_IDLE_STATUS;
WiFiClient  client;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

void setup() {
	Serial.begin(115200);
	delay(10);

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());


	ThingSpeak.begin(client);
	sensors.begin();

	// Grab a count of devices on the wire
	numberOfDevices = sensors.getDeviceCount();

	// locate devices on the bus
	Serial.print("Locating devices...");

	Serial.print("Found ");
	Serial.print(numberOfDevices, DEC);
	Serial.println(" devices.");

	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (sensors.isParasitePowerMode()) Serial.println("ON");
	else Serial.println("OFF");

	// Loop through each device, print out address
	for (int i = 0; i<numberOfDevices; i++)
	{
		// Search the wire for address
		if (sensors.getAddress(tempDeviceAddress, i))
		{
			Serial.print("Found device ");
			Serial.print(i, DEC);
			Serial.print(" with address: ");
			printAddress(tempDeviceAddress);
			Serial.println();

			Serial.print("Setting resolution to ");
			Serial.println(TEMPERATURE_PRECISION, DEC);

			// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
			sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

			Serial.print("Resolution actually set to: ");
			Serial.print(sensors.getResolution(tempDeviceAddress), DEC);
			Serial.println();
		}
		else {
			Serial.print("Found ghost device at ");
			Serial.print(i, DEC);
			Serial.print(" but could not detect address. Check power and cabling");
		}
	}
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}

void loop() {
	sensors.requestTemperatures();
	// Itterate through each sensor and send readings to ThinkSpeak.  This maps sensor 1 to
	// field 1 and sensor 2 to field 2 etc...
	for (int i = 0; i <= (numberOfDevices - 1); i++) {
		float temp = sensors.getTempCByIndex(i);
		ThingSpeak.setField(i + fieldStart, temp);
		Serial.println("Sensor #:");
		Serial.println(i);
		Serial.println("Temperature:");
		Serial.println(temp);
	}
	ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  //Write fields to Thingspeak, comment this line out
															 //if you wish to test without uploading data.
	Serial.println("Data sent to ThinkSpeak");
	delay(updatePeriod * 1000);
}
