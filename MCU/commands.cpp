#include "commands.h"
#include "TaskSlot.h"
#include "Lib485.h"
#include "Lib595.h"
#include "motor.h"
#include "RotSensor.h"
#include "String.h"
extern struct Serial485* p485;
extern struct Motor* pmotor;
extern struct RotSensor* prot_sensor;
extern bool is_calibrating;
extern void begin_auto_calibrate();
// YAESU GS232 compatible commands
// callback functions of each command
const  char *commands[N_COMM] = {
    "R", // Clockwise
    "L", // Counter Clockwise
    "A", // Stop azimuthal rotation
    "C", // Get Direction value (degree)
    "M", // Rotate to a set direction Mxxx
    "S", // All stop
    "F", // full scale calibration
};
bool (*command_calls[N_COMM])(int, char **) = {
    CW_command,
    CCW_command,
    stop_azi_rotation_command,
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

bool stop_azi_rotation_command(int argc, char **argv)
{
    set_task(&task_slot, NULL_TASK, 0, 0);
    return true;
}

bool read_direction_command(int argc, char **argv)
{
    int degree = prot_sensor->get_degree();
    delay(10);
    while(degree < 0)
        degree += 360;
    sprintf(p485->tx_buffer, "%03d", degree); 
    send_serial485(p485, p485->tx_buffer);
    return true;
}

bool rotate_to_command(int argc, char **argv)
{
    int to = String(argv[1]).toInt();
    if (to >= 0 && to <= 450)
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