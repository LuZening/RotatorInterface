var speeds = [100, 100];
s_task_buffer = "";
manual_time = 2933; // ms
var tk_cw = new Array(2);
var tk_ccw = new Array(2);
var tk_rot = new Array(2);
function create_task_target(motno) {
    if (is_mouseon) {
        s_task_buffer = 'mot='+motno +'&type=' + 2 + '&to=' + Math.round(deg_task) + '&speed=' + speeds[motno-1] + '\n';
    }
}

function send_task_manual(motno, direction) {
    // direction: -:CCW +:CW
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "task", true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    s_temp = 'mot='+motno + '&type=' + 1 + '&to=' + Math.round(direction * manual_time) + '&speed=' + speeds[motno-1] + '\n';
    xhttp.send(s_temp);
}

function send_task_stop(motno) {
    s_task_buffer = "";
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "task", true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    s_temp =  'mot='+motno +'&type=' + 0 + '&to=' + 1 + '&speed=' + 1 + '\n';
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

function on_mousedown_CW(motno)
{
    clearInterval(tk_rot[motno-1]);
    send_task_manual(motno,1);
    tk_rot[motno-1] = setInterval(function(){send_task_manual(motno, 1);}, manual_time / 3);
}

function on_mousedown_CCW(motno)
{
    clearInterval(tk_rot[motno-1]);
    send_task_manual(motno,-1);
    tk_rot[motno-1] = setInterval(function(){send_task_manual(motno, -1);}, manual_time / 3 );
}


function on_mouseup_manual(motno)
{
    clearInterval(tk_rot[motno-1]);
    for(i=0;i<2;++i)
    {
       send_task_stop(motno);
    }
}

function test_onclick(n)
{
    console.log("clicked " + n);
}