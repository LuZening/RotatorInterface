#include "Lib485.h"
struct Serial485 _serial485;
struct Serial485 *p485 = &_serial485;
#ifdef GS232
int compatibility = COMPATIBILITY_GS232;
#else
int compatibility = COMPATIBILITY_OWN;
#endif
// Compliance with CRC16 MODBUS procotols
uint CRC16(char *s, int len) // chr_termin excluded
{
    unsigned int CRC_reg = 0xffff; // initialize 16-bit CRC register
    while (len-- && (*s > 0))
    {
        CRC_reg = CRC_reg ^ *s;
        s++;
        for (int i = 0; i < 8; ++i)
        {
            if (CRC_reg & 0x1) // shift 1
            {
                CRC_reg >>= 1;
                CRC_reg ^= CRC_POLY;
            }
            else // shift 0
                CRC_reg >>= 1;
        }
    }
    return CRC_reg;
}
void begin_serial485(struct Serial485 *p485, HardwareSerial *pSerial, uint baud, int pin_RW)
{
    int i;
    p485->state_RW = RE;
    p485->baud = baud;
    p485->timeout_tx = 0;
    p485->idx_command = 0;
    *(p485->command) = 0;
    p485->argc = 0;
    p485->is_command_ready = false;
    p485->timeout_clear_rx = TIMEOUT_RX;
    p485->n_available = 0;
    p485->pSerial = pSerial;
    p485->pSerial->begin(baud);
    p485->pin_RW = pin_RW;
    digitalWrite(pin_RW, RE);
}

void send_serial485(struct Serial485 *p485, char *buffer_send)
{
    p485->state_RW = DE;
    digitalWrite(p485->pin_RW, DE);
    delayMicroseconds(200);
    p485->pSerial->write(buffer_send);
    // the routine will not clear the RW/DE register
    // cleaning will be done by repeaded timer routine
    p485->timeout_tx = (strlen(buffer_send) + 4) * 10 * 1000 / p485->baud + 1;
}

// call this function at each interval
// switches DE to RE when transmission finished
void handle_serial485(struct Serial485 *p485)
{
    static unsigned int n_available_prev = 0;
    // when TX timeout, we can clear the DE signal on the RW control pin
    if (p485->timeout_tx > 0)
        p485->timeout_tx -= CHK_INTERVAL_485;
    else if (p485->state_RW == DE && p485->timeout_tx <= 0)
    {
        p485->pSerial->flush();
        p485->timeout_tx = 0;
        digitalWrite(p485->pin_RW, RE);
        p485->state_RW = RE;
    }
    // if there are new income data to the buffer, reset counting down
    if (p485->pSerial->available() > n_available_prev)
    {
        p485->timeout_clear_rx = TIMEOUT_RX;
    }
    // when RX buffer timeout, clear the RX buffer
    if (p485->timeout_clear_rx > 0)
    {
        p485->timeout_clear_rx -= CHK_INTERVAL_485;
        if (p485->timeout_clear_rx <= 0)
        {
            // clear the RX buffer
            while (p485->pSerial->available())
                p485->pSerial->read();
            p485->idx_command = 0;
            p485->command[0] = 0;
            p485->n_available = 0;
        }
    }
    // read byte from the RX Buffer
    while (p485->pSerial->available())
    {
        char c = p485->pSerial->read();
        // if c is a deliminator, we have a complete command
        if (strchr(DELIM_485, c) != NULL)
        {
            p485->command[(p485->idx_command)++] = 0;
            // eliminate the trailling deliminating characters
            while (p485->pSerial->available() && strchr(DELIM_485, p485->pSerial->peek()) != NULL)
                p485->pSerial->read();
            parse_command(p485);
            break;
        }
        else
        {
            p485->command[(p485->idx_command)++] = c;
        }
    }
    p485->n_available = p485->pSerial->available();
    // execute command if command has been parsed
    if(p485->is_command_ready)
    {
        if (!execute_command(p485->argc,(char**)(p485->argv)))
        {
            send_serial485(p485, "bad command\r\n");
        }
        // reset the command_ready flag
        p485->is_command_ready = false;
        p485->idx_command = 0;
    }
}

void parse_command(struct Serial485 *p485)
{
    char *command = p485->command;
    char *tok;
    p485->argc = 0;
    switch (compatibility)
    {
    case COMPATIBILITY_GS232:
        p485->argv[0][0] = command[0];
        p485->argv[0][1] = 0;
        p485->argc = 1;
        tok = strtok(command + 1, DELIM_COMM_485);
        while (tok)
        {
            p485->argv[(p485->argc)++] = tok;
            tok = strtok(NULL, DELIM_COMM_485);
        }
    default:
        tok = strtok(command, DELIM_COMM_485);
        while (tok)
        {
            p485->argv[(p485->argc)++] = tok;
            tok = strtok(NULL, DELIM_COMM_485);
        }
    }
    p485->is_command_ready = true;
}

bool execute_command(int argc, char **argv)
{
    int i;
    // search in command list
    for (i = 0; i < N_PARAM_MAX; ++i)
    {
        if (strcmp(argv[0], commands[i]) == 0) // if any of the listed commands matches
        {
            return command_calls[i](argc, argv);
        }
    }
    return false;
}