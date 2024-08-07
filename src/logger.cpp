#include <logger.h>
#include <panel_data.h>


void log_array(uint8_t *array, uint8_t len)
{
  Serial.printf("%02X", array[0]);

  for (uint8_t i = 1; i < len; ++i)
  {
    Serial.printf("-%02X", array[i]);
  }
  Serial.println(); // Print a newline at the end
}

void log_inverter(Inverter *inverter)
{
  Serial.printf_P(PSTR("Serial: %02X-%02X-%02X-%02X-%02X-%02X\n"), inverter->serial[0], inverter->serial[1], inverter->serial[2], inverter->serial[3], inverter->serial[4], inverter->serial[5]);
  Serial.printf_P(PSTR("ID: %02X-%02X\n"), inverter->iD[0], inverter->iD[1]);
  Serial.printf_P(PSTR("Type: %i\n"), inverter->type);
  Serial.printf_P(PSTR("Idx: %i\n"), inverter->idx);

  Serial.printf_P(PSTR("Polled: %i\n"), inverter->polled);

  Serial.printf_P(PSTR("TimeStamp: %i\n"), inverter->timeStamp);
  
  Serial.printf_P(PSTR("Power: %iW\n"), inverter->acPower);

  Serial.printf_P(PSTR("Frequency: %.2fHz\n"), inverter->frequency);

  Serial.printf_P(PSTR("Temperature: %.2f°C\n"), inverter->temperature);
  
  Serial.printf_P(PSTR("AC Voltage: %.2fV\n"), inverter->acVoltage);
  Serial.printf_P(PSTR("MPTT: %.2fV\n"), inverter->dcMpttVoltage);

  //0x00 (both panel power generate), 0x02 (under/overload?), 0x03 (no power generate), 0x04 (???), 0x05 (only DC2 power generate), 0x06 (only DC1 power generate), 0x0b (boot up?), 0x0c (boot up?)
  switch(inverter->status)
  {
    case 0x00:
      Serial.println(F("Both power generate"));
      break;
    case 0x02:
      Serial.println(F("Overload"));
      break;
    case 0x03:
      Serial.println(F("No power generate"));
      break;
    case 0x04:
      Serial.println(F("???"));
      break;
    case 0x05:
      Serial.println(F("Only DC2 power generate"));
      break;
    case 0x06:
      Serial.println(F("Only DC1 power generate"));
      break;
    case 0x0b:
      Serial.println(F("Boot up 1"));
      break;
    case 0x0c:
      Serial.println(F("Boot up 2"));
      break;
      default:
      Serial.printf_P(PSTR("Status: %02X\n"), inverter->status);
      break;

  }
  // for each inverter print data
  for (uint8_t i = 0; i < 2; i++)
  {
    Serial.printf_P(PSTR("****** Panel %i ******\n"), i);
    Serial.printf_P(PSTR("DC current : %.2f\n"), inverter->panels[i].dcCurrent);
    Serial.printf_P(PSTR("DC voltage : %.2f\n"), inverter->panels[i].dcVoltage);
    Serial.printf_P(PSTR("Energy : %.2f\n"), inverter->panels[i].energy);
    Serial.println("************");
  }
}

void log_total(Inverter *inverter, uint8_t len)
{
  float totalPower = 0;
  float totalEnergy = 0;
  for (uint8_t i = 0; i < len; i++)
  {
    totalPower += inverter[i].acPower;
    totalEnergy += inverter[i].energy;
  }

  Serial.printf_P(PSTR("Total Power: %.2fW\n"), totalPower);
  Serial.printf_P(PSTR("Total Energy: %.2fWh\n"), totalEnergy);
}