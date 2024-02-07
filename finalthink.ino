//rfid headers
#include <SPI.h>      //include the SPI library
#include <MFRC522.h>  //include the MFRC522 RFID reader library
#define RST_PIN 9  //reset pin, which can be changed to another digital pin if needed.
#define SS_PIN 10  //SS or the slave select pin, which can be changed to another digital pin if needed.
MFRC522 mfrc522(SS_PIN, RST_PIN);  // create a MFRC522 instant.
MFRC522::MIFARE_Key key;          //create a MIFARE_Key struct named 'key' to hold the card information
byte readbackblock[18];  //Array for reading out a block.
byte read[7];
//finger scanner headers
#include <Adafruit_Fingerprint.h>
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
//espcommunication headers
#include <SoftwareSerial.h>
#include <Wire.h>
//espserial creation
SoftwareSerial mySerial1(4, 5); // RX, TX
void setup()
{
  //serialmonitor initialization
  Serial.begin(115200);        // Initialize serial communications with the PC
  while(!Serial);
  //rfid initialization
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card (in case you wonder what PCD means: proximity coupling device)
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;  // Prepare the security key for the read and write operations.
  }
  mfrc522.PCD_DumpVersionToSerial();
  //esp communication setup
  mySerial1.begin(9600);
  while(!mySerial1);
  //fingerprint 
  finger.begin(57600);
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); 
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); 
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); 
  Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); 
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
  finger.getTemplateCount();
  if (finger.templateCount == 0) 
  {
    Serial.print("Sensor doesn't contain any fingerprint data.");
  }
  else 
  {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); 
    Serial.print(finger.templateCount); 
    Serial.println(" templates");
  }
  pinMode(8,OUTPUT);
}
void loop()
{
  // Look for new cards if not found rerun the loop function
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // read from the card if not found rerun the loop function
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  Serial.println("Scan a MIFARE Classic card");
  readBlock(2, readbackblock);  //read block 2
  readBlock(4, read);  //read block 4
  //print data
  Serial.print("read block 2: ");
  String tagID = byteArrayToString(readbackblock, 19);
  String arr[5];
  String rs="";
  int x=0;
  for(int i=0;i<tagID.length();i++)
  {
    if(tagID[i]==',')
    {
      arr[x++]=rs;
      rs="";
    }
    else
      rs+=tagID[i];
  }
  arr[x++] = rs;
 for(int i=0;i<5;i++)
 {
  Serial.println(arr[i]);
 }
  Serial.println();
  bool matchFound = false;
  // Perform fingerprint verification
  String fp = (String)getFingerprintID();
  //(4, read);  //read block 2
  //print data
  for (int i = 0; i < 5; i++) {
  //byte readbackblock[10];
  //readBlock(blocksToRead[i], readbackblock);
  // Concatenate block data to tagID
  //tagID += byteArrayToString(readbackblock, 3);
  Serial.println("tag:"+arr[i]);
  Serial.println("fp: "+fp);
  // Compare tagID with fp
  if (arr[i] == fp) {
    matchFound = true;
    Serial.println("validated");
    digitalWrite(8, HIGH);
    break; // Exit the loop if a match is found
  }
  else
  {
    Serial.println("tagid: "+arr[i]+"and fp id: "+fp+" are not matched");
  }
  // Clear tagID for the next iteration
  tagID = "";
  delay(100); // Adjust the delay between blocks as needed
  }
  if(!matchFound)
  {
    for (int j = 0 ; j < 7 ; j++)
    {
      mySerial1.print((char)read[j]);
    }
    mySerial1.flush();
  }
}
//Read specific block
int readBlock(int blockNumber, byte arrayAddress[])
{
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector
  //authentication of the desired block for access
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed : ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 3;//return "3" as error message
  }
  //reading data from the block
  byte buffersize = 18;
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Data read failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;//return "4" as error message
  }
  Serial.println("Data read successfully");
}
//finding finger index in the finger memory
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  // found a match!
  Serial.print("Found ID #"); 
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence);
  return finger.fingerID;// returns p if failed, otherwise returns ID #
}
//conversion to string
String byteArrayToString(byte* byteArray, int length) {
  String result = "";
  for (int i = 0; i < length; i++) {
    result += (char)byteArray[i];
  }
  return result;
}
