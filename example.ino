#include <Wire.h>
#include <MCP23017.h>

MCP23017 mcp(0x20);

const int NUM_RESISTORS = 8;
const int resistorValues[NUM_RESISTORS] = {1, 2, 5, 10, 20, 39, 82, 160};

// Spiegelt de bits van een byte (bit0 <-> bit7, bit1 <-> bit6, etc.)
uint8_t reverseBits(uint8_t b) {
  uint8_t result = 0;
  for (int i = 0; i < 8; i++) {
    result |= ((b >> i) & 1) << (7 - i);
  }
  return result;
}

int findNearestResistance(int target) {
  int bestCombo = 0;
  int bestValue = 0;
  int bestDiff = abs(target - 0);

  for (int combo = 0; combo < 256; combo++) {
    int total = 0;
    for (int bit = 0; bit < NUM_RESISTORS; bit++) {
      if (combo & (1 << bit)) {
        total += resistorValues[bit];
      }
    }
    int diff = abs(target - total);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestCombo = combo;
      bestValue = total;
    }
  }

  Serial.print("Gevraagd: ");
  Serial.print(target);
  Serial.print(" ohm  ->  Dichtstbijzijnde: ");
  Serial.print(bestValue);
  Serial.print(" ohm  |  Actieve weerstanden: ");
  for (int bit = 0; bit < NUM_RESISTORS; bit++) {
    if (bestCombo & (1 << bit)) {
      Serial.print(resistorValues[bit]); Serial.print(", ");
    }
  }
  Serial.println();

  return bestCombo;
}

void WriteResistorValue(int ohms) {
  int combo = findNearestResistance(ohms);

  // GPA = bypass, gespiegeld (GPA0 hoort bij GPB7)
  // GPB = weerstand actief, gespiegeld (GPB0 hoort bij GPA7)
  uint8_t gpbValue = combo;          // GPB = weerstanden actief
  uint8_t gpaValue = reverseBits(~combo & 0xFF);  // GPA = bypass actief, reversed

  mcp.writePort(MCP23017Port::A, gpaValue);
  mcp.writePort(MCP23017Port::B, gpbValue);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mcp.init();
  mcp.portMode(MCP23017Port::A, 0);
  mcp.portMode(MCP23017Port::B, 0);

  Serial.println("MCP23017 geinitialiseerd");
  WriteResistorValue(0);
  delay(2000);
}

void loop() {
  int val = analogRead(A1);
  int rval = map(val, 0,1023,0,255);
  WriteResistorValue(rval);
  
  delay(200);
}
