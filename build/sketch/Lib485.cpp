#line 1 "e:\\Projects\\RADIO\\Projects\\RotatorInterface\\MCU\\Lib485.cpp"
#include "Lib485.h"
#include "stdlib.h"
#include "commands.h"
struct Serial485 _serial485;
struct Serial485 *p485 = &_serial485;
#define OWN 0
#define GS232 1
#define COMPATIBILITY GS232
//#define COMPATIBILITY OWN
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
void begin_serial485(struct Serial485 *p485, HardwareSerial *pSerial, uint baud, int pin_RW, int timer_ms)
{
    int i;
    p485->state_RW = RE;
    p485->baud = baud;
    p485->timeout_tx = 0;
    p485->idx_command = 0;
    *(p485->command) = 0;
    p485->argc = 0;
    p485->argv[0] = p485->argv0;
    p485->is_command_ready = false;
    p485->timer_ms = timer_ms;
    p485->timeout_clear_rx = TIMEOUT_RX;
    p485->n_available = 0;
    p485->pSerial = pSerial;
    p485->pSerial->begin(baud);
    p485->pin_RW = pin_RW;
    digitalWrite(pin_RW, RE);
}

void send_serial485(struct Serial485 *p485, const char *str)
{
    p485->state_RW = DE;
    int len_content = strlen(str);

    digitalWrite(p485->pin_RW, DE);
    p485->pSerial->write(str);
    // delay(20);
    // the routine will not clear the RW/DE register
    // cleaning will be done by repeaded timer routine
    p485->timeout_tx += (len_content + ((len_content > 20) ? (20) : (len_content))) * 10 * 1000 / p485->baud + 1;
}

// call this function at each interval
// switches DE to RE when transmission finished
void handle_serial485(struct Serial485 *p485)
{
    // tx timeout timer
    if (p485->state_RW == DE)
    {
        if (p485->timeout_tx > 0)
        {
            p485->timeout_tx -= p485->timer_ms;
        }
        // when TX timeout, clear the DE signal on the RW control pin
        else
        {
            p485->timeout_tx = 0;
            digitalWrite(p485->pin_RW, RE);
            p485->state_RW = RE;
        }
    }
    // read a byte from the RX Buffer
    else // RE
    {
        while (p485->pSerial->available())
        {
            p485->timeout_clear_rx = TIMEOUT_RX; // reset rx clear timer
            char c = p485->pSerial->read();
            // if c is a deliminator, we have a complete command
            if (strchr(DELIM_485, c) != NULL) // c is a deliminator
            {
                if (p485->idx_command > 0) // parse the command if the command buffer is not empty
                {
                    p485->command[p485->idx_command] = 0;
                    // eliminate the trailling deliminating characters
                    while (p485->pSerial->available() && strchr(DELIM_485, p485->pSerial->peek()) != NULL)
                        p485->pSerial->read();
                    parse_command(p485);
                    // execute command if a command has been parsed
                    if (p485->is_command_ready)
                    {
                        // delay(1);
                        if (execute_command(p485->argc, (char **)(p485->argv)))
                        {
                            send_serial485(p485, "\r"); // succeeded
                        }
                        else
                        {
                            send_serial485(p485, "?>\r"); // bad command
                        }
                        // clean up the command flags
                        p485->is_command_ready = false;
                        p485->idx_command = 0;
                        p485->argc = 0;
                    }
                } // discard the deliminator if the command is null
            }
            else
            {
                // avoid command buffer overflow
                if (p485->idx_command < COMM_BUFFER_SIZE)
                    p485->command[(p485->idx_command)++] = c;
                else // wrapback if overflow
                    p485->idx_command = 0;
            }
        }
        // countdown rx clear timer
        if (p485->timeout_clear_rx > 0)
        {
            p485->timeout_clear_rx -= p485->timer_ms;
            if (p485->timeout_clear_rx <= 0)
            {
                p485->idx_command = 0; // clear the rx buffer
                p485->command[0] = 0;
            }
        }
    }
}

void parse_command(struct Serial485 *p485)
{
    char *command = p485->command;
    char *tok;
    p485->argc = 0;
#if COMPATIBILITY == GS232
    p485->argv[0][0] = command[0];
    if (p485->idx_command > 1 &&
        command[0] != 'M' && command[1] >= '0' && command[1] <= '9') // handle cases like O2
    {
        p485->argv[0][1] = command[1];
        p485->argv[0][2] = 0;
        command+=2;
    }
    else
    {
        p485->argv[0][1] = 0;
        command++;
    }
    p485->argc = 1;
    tok = strtok(command, DELIM_COMM_485);
    while (tok)
    {
        p485->argv[(p485->argc)++] = tok;
        tok = strtok(NULL, DELIM_COMM_485);
    }
#elif COMPATIBILITY == OWN
    tok = strtok(command, DELIM_COMM_485);
    while (tok)
    {
        p485->argv[(p485->argc)++] = tok;
        tok = strtok(NULL, DELIM_COMM_485);
    }
#endif
    p485->is_active = true;
    p485->is_command_ready = true;
}

bool execute_command(int argc, char **argv)
{
    int i;
    bool res = false;
    // search in command list
    for (i = 0; i < N_COMM; ++i)
    {
        if (strcmp(argv[0], commands[i]) == 0) // if any of the listed commands matches
        {
            return (*command_calls[i])(argc, argv);
        }
    }
    return false;
}

uint8_t hexchr2num(char c)
{
    uint8_t n = 0;
    if(c >= '0' && c <= '9')
    {
        n = c - '0';
    }
    else if(c >= 'A' && c <= 'Z')
    {
        n = c - 'A' + 10;
    }
    else if(c >= 'a' && c<= 'z')
    {
        n = c - 'a' + 10;
    }
    return n;
}