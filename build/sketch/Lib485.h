/**** Library for 485 communications *******/

#ifndef __LIB485_H_
#define __LIB485_H_

#include <Arduino.h>
#define CRC_POLY 0xA001
#define RE HIGH
#define DE LOW
#define COMM_BUFFER_SIZE 64
#define N_PARAM_MAX 8
#define PARAM_LEN_MAX 5
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
    char argv0[4]; // reserve memory for argv[0]
    bool is_command_ready;
    BYTE state_RW;
    uint baud;
    HardwareSerial *pSerial;
    unsigned int pin_RW;
    int timer_ms; // timer interval
    int timeout_tx;       // estimated transmission time in ms
    int timeout_clear_rx; // time to clear RX buffer
    unsigned int n_available;
    bool is_active = false; // indicate if the 485 port is in use
};

extern struct Serial485 *p485;

uint CRC16(char *s, int len); // chr_termin excluded
void begin_serial485(struct Serial485 *p485, HardwareSerial *pSerial, uint baud, int pin_RW, int timer_ms);
void send_serial485(struct Serial485 *p485, const char *buffer_send);
void handle_serial485(struct Serial485 *p485);
void parse_command(struct Serial485 *p485);
bool execute_command(int argc, char **argv);



#endif
