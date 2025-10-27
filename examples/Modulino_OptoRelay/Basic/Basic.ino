#include <Modulino.h>

ModulinoOptoRelay relay;

void setup() {
  Serial.begin(115200);
  Modulino.begin();
  relay.begin();

}
void loop() {
  relay.on();
  if(relay.getStatus())
  {
    Serial.println("Relay state ON!");
  }
  delay(1000);
  relay.off();
  if(!(relay.getStatus()))
  {
    Serial.println("Relay state OFF!");
  }
  Serial.println();
  delay(1000);
}