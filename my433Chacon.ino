#include <MySensor.h>
#include <SPI.h>
#include <NewRemoteReceiver.h>
#include <NewRemoteTransmitter.h>

#define RF_RX_PIN 1
#define RF_TX_PIN 5

#define RF433_RECEIVE_ID 1

#define FIRST_TRANSMIT_ID 10
#define NUMBER_OF_TRANSMIT 4 // Total number of attached relays


MySensor gw;
MyMessage msg(RF433_RECEIVE_ID, V_VAR1);

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 100;    // the debounce time; increase if the output flickers


void setup()
{
	// See the interrupt-parameter of attachInterrupt for possible values (and pins)
  	// to connect the receiver.
  	NewRemoteReceiver::init(RF_RX_PIN, 1, my433Chacon_Receive);	
  
	// Startup and initialize MySensors library. Set callback for incoming messages.
	gw.begin(my433Chacon_Send, AUTO, false);
	// Send the sketch version information to the gateway and Controller
	gw.sendSketchInfo("433 Gateway", "1.0");
	gw.present(RF433_RECEIVE_ID, S_IR);

  	// Fetch relay status
  	for (int sensor = 0 ; sensor < NUMBER_OF_TRANSMIT ; sensor++)
   	{
    	// Register all sensors to gw (they will be created as child devices)
      	gw.present(FIRST_TRANSMIT_ID + sensor, S_LIGHT);
  	}

  	lastDebounceTime=millis();
}


void loop()
{
	// You can do other stuff here!
	gw.process();
}



void my433Chacon_Receive(NewRemoteCode receivedCode) 
{
	long data ; 
	char mymsg[10];

	if ((millis() - lastDebounceTime) > debounceDelay)
	{
		// Print the received code.
	  	Serial.print("Addr: ");
	  	Serial.print(receivedCode.address);
	  	Serial.print(", period: ");
	  	Serial.print(receivedCode.period);
	  	Serial.print("us");
	 	Serial.print(", GroupBit: ");
	  	Serial.print(receivedCode.groupBit);
	   	Serial.print(", Type: "); 
	    Serial.print(receivedCode.switchType);
	    Serial.print(", Unit: ");
	    Serial.println(receivedCode.unit);

		data = receivedCode.address ;
		data = data * 100L ; 
		data = data + (10 * receivedCode.unit) ; 
		data = data + receivedCode.switchType ; 
		Serial.print("data : ");
		Serial.println(data);

		ltoa(data,mymsg,10);
		Serial.println(mymsg);
		msg.set(mymsg);
		gw.send(msg);

		lastDebounceTime=millis();
	}
}


void my433Chacon_Send(const MyMessage &message)
{
	long data ; 
	long code ; 
	int val ;
	int unit ;

	if (message.type==V_LIGHT)
	{
		Serial.print("data : ");
		Serial.println(message.data);

		data = atol(message.data);

		code = data / 100L ; 
		unit =  (data % 100L) / 10L ;
		val = data % 10L ; 

		Serial.print("code :");
		Serial.print(code);
		Serial.print(", unit : ");
		Serial.print(unit);
		Serial.print(", val : ");
		Serial.println(val);

     	// Disable the receiver; otherwise it might pick up the retransmit as well.
  		NewRemoteReceiver::disable();
  		NewRemoteTransmitter transmitter(code, 5, 266);
        transmitter.sendUnit(unit, val);
    	NewRemoteReceiver::enable();
 	}
}




