#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

void setup(void) {
  Serial.begin(115200);
 // pinMode(19,OUTPUT);
  Serial.println("NDEF Reader");
  nfc.begin();
}


void loop(void) {

  if (nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    //tag.print();
    String uid = tag.getUidString();
    Serial.println(uid);
    /*
    digitalWrite(19,HIGH);
    delay(2000);
    digitalWrite(19,LOW);*/
  }
  delay(100);
}
