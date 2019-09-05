const ws_uri = 'ws://' + location.hostname + '/ws';
// create WS object
var ws_conn = new WebSocket(ws_uri, ['arduino']);
/***** config WS event handlers */
var ws_heartCheck = {
    timeout: 3000,//3000ms
    timeoutObj: null,
    reset: function(){
        clearTimeout(this.timeoutObj);
        this.start();
    },
    start: function(){
        this.timeoutObj = setTimeout(function(){
            ws_conn.send("ack");
        }, this.timeout)
    }
}
ws_conn.onopen = function () {
    ws_heartCheck.start();
};
ws_conn.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
ws_conn.onclose = function () {
    console.log('WebSocket ws_conn closed');
    ws_conn = new WebSocket(ws_uri, ['arduino']); // reconnect
    console.log('WebSocket reconnected')
};
ws_conn.onmessage = function (e) {
    msg = e.data;
    ws_heartCheck.reset();
    if (msg.includes("azu")) // Sensor Data
    {
        parseSensorData(msg);
    }
};

function update_bulb(azu, lmt) {
    ele = document.getElementById("bulb");
    if (ele) {
        if (lmt == 1) {
            ele.style["background-color"] = "red";
        }
        else if ((azu < 0 || azu > 360)) {
            ele.style["background-color"] = "orange";
        }
        else {
            ele.style["background-color"] = "cyan";
        }
    }
}

var azu_orig

function azu_onupdatepos(deg)
{
    pos_fan = deg;
}
function parseSensorData(s) {
    // parse the recieved text
    // azu=\d+&ADC=\d+&busy=\d+&lmt=\d+\n
    s_params = s.match(/-*\d+/g);
    azu_orig = Number(s_params[0]);
    azu = azu_orig % 360;
    if (azu < 0) azu += 360;
    ADC = s_params[1];
    busy = s_params[2];
    lmt = s_params[3];

    e = document.getElementById("deg");
    if (e) e.innerHTML = azu;
    e = document.getElementById("ADC");
    if (e) e.innerHTML = ADC;
    e = document.getElementById("limit");
    if (e) e.innterHTML = lmt;
    update_bulb(azu_orig, lmt);
    azu_onupdatepos(azu);
}

function getSensorData() // request sensor data by AJAX
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.status == 200 && this.responseText.length > 0) {
            parseSensorData(this.responseText);
        }
    }
    xhttp.open("GET", "getSensor", true);
    xhttp.send();
}
