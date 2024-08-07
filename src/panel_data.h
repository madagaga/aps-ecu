#ifndef panel_data_h_
#define panel_data_h_

#include <stdint.h>

#define DS3_DC_VOLTAGE_FACTOR (float)1/48;
#define DS3_DC_CURRENT_FACTOR 0.0125;
#define DS3_AC_VOLTAGE_FACTOR 3.8;

typedef enum InverterType
{
    YC600 = 1, 
    QS1,
    DS3
} InverterType;


typedef struct 
{
    float dcCurrent = 0; // ampere
    float dcVoltage = 0; // volt   
    float energy = 0; // Wh
    bool present = false;
} PanelData;

typedef struct
{    
    uint8_t serial[6] = {0};
    uint8_t iD[2] = {0};
    InverterType type = DS3;
    uint8_t idx = 0;  
    bool paired = false;  
    bool polled = false;
    int timeStamp = 0;
    int acPower = 0; // W
    float energy = 0; // Wh
    float frequency = 0; // Hz";
    float temperature = 0;
    float acVoltage = 0; //v 
    float dcMpttVoltage = 0; // V
    uint8_t status = 0;
    PanelData panels[4];
    uint8_t _poll_command[31];
    
} Inverter;


#endif /* panel_data_h_ */