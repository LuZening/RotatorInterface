speed = 255;
s_task_buffer = "";
manual_time = 1000; // ms
var tk_cw;
var tk_ccw;
var tk_rot;
function create_task_target() {
    if (is_mouseon) {
        s_task_buffer = 'type=' + 2 + '&to=' + Math.round(deg_task) + '&speed=' + speed + '\n';
    }
}

function send_task_manual(direction) {
    // direction: -:CCW +:CW
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "task", true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    s_temp = 'type=' + 1 + '&to=' + Math.round(direction * manual_time) + '&speed=' + speed + '\n';
    xhttp.send(s_temp);
}

function send_task_stop() {
    s_task_buffer = "";
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "task", true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    s_temp = 'type=' + 0 + '&to=' + 1 + '&speed=' + 1 + '\n';
    xhttp.send(s_temp);
}

function send_task() {
    if (s_task_buffer.length > 0) {
        var xhttp = new XMLHttpRequest();
        xhttp.open("POST", "task", true);
        xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        xhttp.send(s_task_buffer);
        s_task_buffer = "";
    }
}

function on_mousedown_CW()
{
    clearInterval(tk_rot);
    send_task_manual(2);
    tk_rot = setInterval(function(){send_task_manual(2);}, 1000);
}

function on_mousedown_CCW()
{
    clearInterval(tk_rot);
    send_task_manual(-2);
    tk_rot = setInterval(function(){send_task_manual(-2);}, 1000);
}

function on_mouseup_manual()
{
    clearInterval(tk_rot);
    for(i=0;i<2;++i)
    {
       send_task_stop();
    }
}