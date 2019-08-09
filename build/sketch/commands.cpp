#include "commands.h"
#include "TaskSlot.h"
#include "Lib485.h"
#include "Lib595.h"
#include "motor.h"
#include "RotSensor.h"
extern struct Task task_slot;
extern struct Serial485* p485;
extern struct Motor* pmotor;
extern struct RotSensor* prot_sensor;
extern bool is_calibrating;
extern void begin_auto_calibrate();
// GS232 compatible commands
const char *commands[N_COMM_MAX] = {
    "R", // Clockwise
    "L", // Counter Clockwise
    "A", // Stop rotation
    "C", // Return Direction value (degree)
    "M", // Rotate to a set direction Mxxx
    "S", // All stop
    "F", // Full scale calibration
};
// callback functions of each command
bool (*command_calls[N_COMM_MAX])(int, char **) = {
    CW_command,
    CCW_command,
    stop_rotation_command,
    read_direction_command,
    rotate_to_command,
    all_stop_command,
    full_calib_command};
/************************************************
 *             HANDLE RS485 commands            *
 * *********************************************/

bool CW_command(int argc, char **argv)
{
    set_task(&task_slot, MANUAL, INFINITE_TIME, MAX_SPEED);
    return true;
}

bool CCW_command(int argc, char **argv)
{
    set_task(&task_slot, MANUAL, -INFINITE_TIME, MAX_SPEED);
    return true;
}

bool stop_rotation_command(int argc, char **argv)
{
    set_task(&task_slot, NULL_TASK, 0, 0);
    return true;
}

bool read_direction_command(int argc, char **argv)
{
    sprintf(p485->tx_buffer, "%d", prot_sensor->degree);
    send_serial485(p485, p485->tx_buffer);
    return true;
}

bool rotate_to_command(int argc, char **argv)
{
    int to = atoi(argv[1]);
    if (to >= 0 && to < 360)
    {
        set_task(&task_slot, TARGET, to, MAX_SPEED);
        return true;
    }
    return false;
}

bool all_stop_command(int argc, char **argv)
{
    set_task(&task_slot, NULL, 0, 0);
    return true;
}
bool full_calib_command(int argc, char **argv)
{
    if (!is_calibrating && pmotor->status == MOT_IDLE)
    {
        begin_auto_calibrate();
        return true;
    }
    return false;
}