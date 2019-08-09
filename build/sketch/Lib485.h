/**** Library for 485 communications *******/

#ifndef __LIB485_H_
#define __LIB485_H_

#include <Arduino.h>
#include "stdlib.h"
#include "commands.h"
#define CRC_POLY 0xA001
#define RE HIGH
#define DE LOW
#define COMM_BUFFER_SIZE 512
#define N_PARAM_MAX 16
#define CHK_INTERVAL_485 2
#define TIMEOUT_RX 1000
#define DELIM_485 "\r\n"
#define DELIM_COMM_485 " \r\n\t"

#define COMPATIBILITY_OWN 0
#define COMPATIBILITY_GS232 1


typedef unsigned char BYTE;
typedef unsigned int uint;

extern int compatibility;

struct Serial485
{
    char command[COMM_BUFFER_SIZE];
    char tx_buffer[256];
    int idx_command;
    int argc;
    char *argv[N_PARAM_MAX];
    bool is_command_ready;
    BYTE state_RW;
    uint baud;
    HardwareSerial *pSerial;
    unsigned int pin_RW;
    int timeout_tx;       // estimated transmission time in ms
    int timeout_clear_rx; // time to clear RX buffer
    unsigned int n_available;
};

extern struct Serial485 *p485;

uint CRC16(char *s, int len); // chr_termin excluded
void begin_serial485(struct Serial485 *p485, HardwareSerial *pSerial, uint baud, int pin_RW);
void send_serial485(struct Serial485 *p485, char *buffer_send);
void handle_serial485(struct Serial485 *p485);
void parse_command(struct Serial485 *p485);
bool execute_command(int argc, char **argv);



#endif
