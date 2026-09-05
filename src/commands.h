#ifndef commands_h_
#define commands_h_ 

#include <Arduino.h>

#define ZNP_SOF 0xFE





/* ECU */
// const uint8_t ECU_ID[]{0xD8, 0xA3, 0x01, 0x1B, 0x97, 0x80};         // "D8A3011B9780"
// const uint8_t ECU_ID_SHORT[]{0xD8, 0xA3};                           // "D8A3"
extern const uint8_t ECU_ID_REVERSE[6];
// const uint8_t ECU_ID_REVERSE_SHORT[]{0xA3, 0xD8};                   // "A3D8"

/* INIT COMMANDS */
extern const uint8_t INIT_0_COMMAND[5];
extern const uint8_t INIT_1_COMMAND[3];
extern const uint8_t INIT_2_COMMAND[12];
extern const uint8_t INIT_3_COMMAND[5];
extern const uint8_t INIT_4_COMMAND[6];
extern const uint8_t INIT_5_COMMAND[8];
extern const uint8_t INIT_6_COMMAND[15];
extern const uint8_t INIT_7_COMMAND[2];

extern const uint8_t INIT_8_COMMAND[2];

extern const uint8_t PING_COMMAND[2];

extern const uint8_t HEART_BEAT_COMMAND[2];

//                                 24    02     0F   FF    FF    FF    FF    FF    FF    FF    FF    14    FF    FF    14    0D    02    00    00    0F    11    00                                       FF    FF    10    FF    FF
extern const uint8_t PAIR_1_COMMAND[39];

//                               24    02    0F    FF    FF    FF    FF    FF    FF    FF    FF    14    FF    FF    14    0C    02    01    00    0F    06    00
extern const uint8_t PAIR_2_COMMAND[22];
//                                 24    02    0F    FF    FF    FF    FF    FF    FF    FF    FF    14    FF    FF    14    0F    01    02    00    0F    11    00    S      E    R    I     A      L                   10    FF    FF
extern const uint8_t PAIR_3_COMMAND[39];
//                               24    02    0F    FF    FF    FF    FF    FF    FF    FF    FF    14    FF    FF    14    01    01    03    00    0F    06    00
extern const uint8_t PAIR_4_COMMAND[28];


// firmware version request
//                                                                                                                                             FB    FB    06    DC    00    00    00    00    00    00    E2    FE    FE
// poll request 
//                                                                                                                                             FB    FB    06    BB    00    00    00    00    00    0C1FEFE
//                                 24    01    %s          14    14    06    00    01    00    0F    13                                  %s    FB    FB    06    BB    00    00    00    00    00    00    C1    FE    FE
//                                            I    D       14    14    06    00    01    00    0F    13                                        FB    FB    06    BB    00    00    00    00    00    00    C1    FE    FE
extern const uint8_t POLL_1_COMMAND[31];

//                             24    01    FF    FF    14    14    06    00    01    00    0F    1E                                        FB    FB   11     00    00    0D    60    30    FB    D3    00    00    00    00    00    00    00    00    04    01    02    81    FE    FE
extern const uint8_t NOOP_COMMAND[42];

#endif