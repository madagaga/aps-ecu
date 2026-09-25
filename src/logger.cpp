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

void log_reading(const Inverter *inverter, const Reading *reading)
{
  Serial.printf_P(PSTR("Serial: %02X%02X%02X%02X%02X%02X  addr %02X%02X  model %02X  lqi %u\n"),
                  inverter->serial[0], inverter->serial[1], inverter->serial[2],
                  inverter->serial[3], inverter->serial[4], inverter->serial[5],
                  inverter->addr[0], inverter->addr[1], inverter->model, inverter->lqi);
  Serial.printf_P(PSTR("Power: %uW  reactive %dVAR\n"), reading->acPower_W, reading->reactive_VAR);
  Serial.printf_P(PSTR("AC: %.1fV  %.2fHz\n"), reading->acVoltage_dV / 10.0f, reading->frequency_cHz / 100.0f);
  Serial.printf_P(PSTR("Temperature: %.1fC  counter %us\n"), reading->temperature_dC / 10.0f, reading->counter_s);
  Serial.printf_P(PSTR("Status: %02X %02X %02X %02X %02X  faults %04X\n"),
                  reading->status[0], reading->status[1], reading->status[2],
                  reading->status[3], reading->status[4], reading->faults);

  for (uint8_t i = 0; i < reading->panelCount; i++)
  {
    const PanelReading *p = &reading->panels[i];
    Serial.printf_P(PSTR("Panel %u: %.2fV  %.3fA  %luWh\n"), i,
                    p->voltage_cV / 100.0f, p->current_mA / 1000.0f, (unsigned long)p->energy_Wh);
  }
}
