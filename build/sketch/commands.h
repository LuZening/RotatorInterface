#ifndef __COMMANDS_H_
#define __COMMANDS_H_

// define compatibility level
#define GS232

#define COMM_LEN 16
#define N_COMM_MAX 7


bool CW_command(int argc, char **argv);
bool CCW_command(int argc, char **argv);
bool stop_rotation_command(int argc, char **argv);
bool read_direction_command(int argc, char **argv);
bool rotate_to_command(int argc, char **argv);
bool all_stop_command(int argc, char **argv);
bool full_calib_command(int argc, char **argv);

extern  const char *commands[N_COMM_MAX];
extern bool (*command_calls[N_COMM_MAX])(int, char **);

#endif