const ws_uri = 'ws://' + location.hostname + '/ws';
// create WS object
var ws_conn = new WebSocket(ws_uri, ['arduino']);
/***** config WS event handlers */
ws_conn.onopen = function () {
    ws_conn.send('Connect ' + new Date());
};
ws_conn.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
ws_conn.onclose = function () {
    console.log('WebSocket ws_conn closed');
    ws_conn.close()
};
ws_conn.onmessage = function (e) {
    msg = e.data;
    console.log(msg);
    if (msg.includes("azu")) // Sensor Data
    {
        parseSensorData(msg);
    }
    console.log('Server: ', e.data);
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
        if (this.status == 200) {
            parseSensorData(this.responseText);
        }
    }
    xhttp.open("GET", "getSensor", true);
    xhttp.send();
}

//setInterval(function () { getSensorData(); }, 500);