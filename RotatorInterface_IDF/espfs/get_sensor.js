const ws_uri = 'ws://' + location.hostname + '/ws';
// create WS object
var ws_conn = new WebSocket(ws_uri);
/***** config WS event handlers */
var ws_reconnect = {
    reconnect_interval: 5000, // 5s
    intervalObj: null,
    retry_count: 0,
    reconnect: function (){
        if(this.retry_count > 10)
        {
            console.log('WS reconnect all atempts failed.')
            this.stop();
            return;
        }
        if(ws_conn)
        {
            ws_conn.close();
        }
        this.retry_count ++;
        // make a new ws 
        ws_conn_recon = new WebSocket(ws_conn.url);
        ws_conn_recon.onopen = ws_conn.onopen;
        ws_conn_recon.onerror = ws_conn.onerror;
        ws_conn_recon.onmessage = ws_conn.onmessage;
        ws_conn = ws_conn_recon;
    },
    start: function (){
        this.retry_count = 0;
        this.intervalObj = setInterval(this.reconnect(), this.reconnect_interval);
    },
    stop: function (){clearInterval(this.intervalObj); this.retry_count = 0;}
}

var ws_heartCheck = {
    timeout: 10000,//10s
    intervalObj: null,
    reset: function(){
        clearInterval(this.intervalObj);
        this.start();
    },
    start: function(){
        this.intervalObj = setInterval(function(){
            ws_conn.send("ack");
        }, this.timeout)
    }
}

var ws_fallback = {
    // timeout: 2000, //2s
    // timeoutObj: null,
    interval: 500, // 500ms
    intervalObj: null,
    start: function(){
        this.intervalObj = setInterval(getSensorData(), this.interval);
    },
    stop: function(){
        clearInterval(this.intervalObj);
    }
}

ws_conn.onopen = function () {
    ws_reconnect.stop();
    ws_heartCheck.start();
};

ws_conn.onerror = function (error) {
    console.log('WebSocket Error ', error);
    ws_heartCheck.stop();
    ws_reconnect.start();
};
ws_conn.onclose = function (e) {
    console.log('WebSocket ws_conn closed' + e.code + ' ' + e.reason + ' ' + e.wasClean);
    //ws_fallback.start(); // fallback to http GET method
    ws_reconnect.start();
    //ws_conn = new WebSocket(ws_uri, ['arduino']); // reconnect
    //console.log('WebSocket reconnected')
};

/* Websocket Message processing routine */
ws_conn.onmessage = function (e) {
    msg = e.data;
    ws_heartCheck.reset();
    // Sensor Data
    if (msg.includes("deg")) 
    {
        parse_sensor_data_string_and_update_elements(msg);
    }
    else if(msg.includes("RS485"))
    {
        parseRS485Data(msg);
    }
};

function update_limit_indicator(motno, deg, limit) {
    ele = document.getElementById(`bulb.${motno}`);
    if (ele) {
        if (lmt == 1) {
            ele.style["background-color"] = "red";
        }
        else {
            ele.style["background-color"] = "cyan";
        }
    }
}

var azu_orig;

function azu_onupdatepos(motno, deg)
{
    pos_fan[motno-1] = deg;
}

function parse_sensor_data_string_and_update_elements(s) {
    // num=%d&ADC=%d&deg=%d&busy=%d&limit=%d&speed=%d
    // num = 1 to N_SENSORS
    s_params = s.match(/-*\d+/g);
    motno = parseInt(s_params[0]);
    azu_orig = Number(s_params[2]);
    azu = azu_orig % 360;
    if (azu < 0) azu += 360;
    ADC = parseInt(s_params[1]);
    busy = parseInt(s_params[3]);
    limit = parseInt(s_params[4]);
    speed100 = parseInt(s_params[5]);

    e = document.getElementById(`deg.${motno}`);
    if (e) e.innerHTML = azu;
    e = document.getElementById(`ADC.${motno}`);
    if (e) e.innerHTML = ADC;
    e = document.getElementById(`limit.${motno}`);
    if (e) e.innterHTML = limit;
    update_limit_indicator(motno, azu_orig, limit);
    azu_onupdatepos(motno, azu);
}

function getSensorData() // request sensor data by AJAX
{
    var xhttp = new XMLHttpRequest();
    xhttp.onload = function () {
        if (this.status == 200 && this.responseText.length > 0) {
            parseSensorData(this.responseText);
        }
    }
    xhttp.open("GET", "getSensor", true);
    xhttp.send();
}

// RS485=data
function parseRS485Data(msg)
{
    console.log(msg);
}