function generate_config_elements(nMots)
{

for(motno = 1; motno <= nMots; motno++)
{

var sTemplateConfig =`
<div id="div_config_mot${motno}">
    <div>
        <h1>Motor ${motno}</h1>
        <h2>Electronic Parameters</h2>
        <form id="fm_params${motno}" action="/setconfig" method="POST" enctype="application/x-www-form-urlencoded">
            <table border="1">
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                </tr>
                <tr>
                    <td>Potentiometer wiring type:</td>
                    <td><input type="radio" name="pot_type.${motno}" value=0>2 terminals
                        <input type="radio" name="pot_type.${motno}" value=1>3 terminals<br></td>
                </tr>
                <tr>
                    <td>Inverse Potentiometer Readouts (Normal: CW->ADC++):</td>
                    <td><input type="radio" name="inverse_ADC.${motno}" value=0>Normal
                        <input type="radio" name="inverse_ADC.${motno}" value=1>Inverse<br></td>
                </tr>
                <tr>
                    <td>Enable multiple rounds:</td>
                    <td><input type="radio" name="allow_multi_rounds.${motno}" value=0>Disable
                        <input type="radio" name="allow_multi_rounds.${motno}" value=1>Enable<br></td>
                </tr>
                <tr>
                    <td>Antenna load (break engagement delay):</td>
                    <td><input type="range" id="brake_engage_defer.${motno}" name="brake_engage_defer.${motno}" min=500 max=3000 step=100>
                        <span id="brake_engage_defer_display.${motno}"></span>ms</td>
                    <script>
                        e_break_delay_display = document.getElementById("brake_engage_defer_display.${motno}");
                        e_break_delay = document.getElementById("brake_engage_defer.${motno}");
                        e_break_delay.addEventListener("input", e => {
                            e_break_delay_display.innerText = e_break_delay.value;
                        });
                    </script>
                </tr>
                <tr>
                    <td>Pre-dividing resistor:</td>
                    <td><input type="number" id="R_0.${motno}" name="R_0.${motno}"></td>
                </tr>
                <tr>
                    <td>Zero Elevation resistor:</td>
                    <td><input type="number" id="R_c.${motno}" name="R_c.${motno}"></td>
                </tr>
                <tr>
                    <td>Maximum potentiometer resistance:</td>
                    <td><input type="number" id="R_max.${motno}" name="R_max.${motno}"></td>
                </tr>
                <tr>
                    <td>Minimum potentiometer resistance:</td>
                    <td><span id="R_min.${motno}"></span></td>
                </tr>
                <tr>
                    <td> <input type="submit" value="Save"></td>
                </tr>
            </table>
        </form>
    </div>
    <div>
        <h2>Calibration</h2>
        <div>
            <h3>Position: <span id="deg.${motno}">---</span>&deg;</h3>
            <h3>ADC Value: <span id="ADC.${motno}">---</span></h3>
        </div>
        <div>
            <button class="large_narrow" id="btn_CCW_lock.${motno}">&#x1f510;&lt;&lt;</button>
            <button class="large" id="btn_CCW.${motno}">&#8634;(CCW)</button>
            <button class="large" id="btn_CW.${motno}">(CW)&#8635;</button>
            <button class="large_narrow" id="btn_CW_lock.${motno}">&gt;&gt;&#x1f510;</button>
            <script>
                b = document.getElementById("btn_CCW.${motno}");
                b.addEventListener(buttonPressEvent, e => { on_mousedown_CCW(${motno}); });
                b.addEventListener(buttonReleaseEvent, e => { on_mouseup_manual(${motno}); });
                b = document.getElementById("btn_CW.${motno}");
                b.addEventListener(buttonPressEvent, e => { on_mousedown_CW(${motno}); });
                b.addEventListener(buttonReleaseEvent, e => { on_mouseup_manual(${motno}); });
                var lock_btn_lisener_gen = (
                    function (bt, motno, dir){
                        var pressed = false;
                        return () => {
                            pressed = !pressed; 
                            console.log(bt.id, pressed);
                            if(pressed)
                            { 
                                bt.style.borderStyle="inset";
                                send_task_manual(motno, dir * 20);                                        
                            }
                            else
                            {
                                bt.style.borderStyle="outset";
                                send_task_stop(motno);                                        
                            }
                        };
                    });
                b = document.getElementById("btn_CCW_lock.${motno}");
                b.onclick = (e => {return lock_btn_lisener_gen(e, ${motno}, -1)})(b) ;
                b = document.getElementById("btn_CW_lock.${motno}");
                b.onclick = (e => {return lock_btn_lisener_gen(e, ${motno}, 1)})(b) ;
            </script>
        </div>
        <br>
        <form method="GET" action="/setconfig">
            <input type="input" hidden name="autocalib.${motno}" style="height:0px;width:0px">
            <input type="submit" value="Auto Calibration">
        </form>
        <form method="GET" action="/setconfig">
            <input type="input" hidden name="manualcalib.${motno}" style="height:0px;width:0px">
            <input type="submit" value="Manual Calibration">
        </form>
        <form method="GET" action="/setconfig">
            <input type="input" hidden name="stop.${motno}" style="height:0px;width:0px">
            <input type="submit" value="Stop Calibration">
        </form>
        <form method="GET" action="/setconfig">
            <input type="input" hidden name="clear_rounds.${motno}" style="height:0px;width:0px">
            <input type="submit" value="Clear Rounds">
        </form>
        <h3>Calibration mode: <span id="is_calib.${motno}">OFF</span></h3>
        <h3>Step I</h3>
        <form method="POST" action="/setconfig" enctype="application/x-www-form-urlencoded">
            <table border="1">
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                </tr>
                <tr>
                    <td>ADC max</td>
                    <td><input id="ADC_max.${motno}" name="ADC_max.${motno}" type="number"></td>
                    <td><button type="button" onclick="set_ADC_max(${motno});">Set</button></td>
                </tr>
                <tr>
                    <td>ADC min</td>
                    <td><input id="ADC_min.${motno}" name="ADC_min.${motno}" type="number"></td>
                    <td><button type="button" onclick="set_ADC_min(${motno});">Set</button></td>
                </tr>
                <tr>
                    <td>North (0&deg;)</td>
                    <td><input id="ADC_zero.${motno}" name="ADC_zero.${motno}" type="number"></td>
                    <td><button type="button" onclick="set_ADC_zero(${motno});">Set</button></td>
                </tr>
                <tr>
                    <td>CW(or Upward) Maximum degree value(by default 360&deg;)</td>
                    <td><input id="deg_max.${motno}" name="deg_max.${motno}" type="number"></td>
                </tr>
                <tr>
                    <td>CCW(or Downward) Minimum degree value(by default 0&deg;)</td>
                    <td><input id="deg_min.${motno}" name="deg_min.${motno}" type="number"></td>
                </tr>
            </table>
            <br>
            <input type="submit" value="save" style="height:50px;width:100px">
        </form>
        <h3>Step 2</h3>
        <form method="POST" action="/setconfig" enctype="application/x-www-form-urlencoded">
            <table border="1">
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                </tr>
                <tr>
                    <td>CW Software Limit</td>
                    <td><input id="deg_limit_CW.${motno}" name="deg_limit_CW.${motno}" type="number"></td>
                    <td><button type="button" onclick="set_CW_lim(${motno});">Set</button></td>
                </tr>
                <tr>
                    <td>CCW Software Limit</td>
                    <td><input id="deg_limit_CCW.${motno}" name="deg_limit_CCW.${motno}" type="number"></td>
                    <td><button type="button" onclick="set_CCW_lim(${motno});">Set</button></td>
                </tr>
            </table>
            <br>
            <input type="submit" value="save" style="height:50px;width:100px">
        </form>
    </div>
</div>
<hr>
`
document.write(sTemplateConfig);

}

}

// retrieve current configurations
function get_config() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        s_recv = this.responseText;
        if (this.status == 200 && s_recv.length > 2) {
            // parse the recieved text
            // name=%s&ssid=%s&ip=%s&pot_type=%d&ADC_min=%d&ADC_max=%d&degree=%d
            /* Parse received XEncoding String */
            // split by &
            Tokens = s_recv.split("&");
            for(i = 0; i < Tokens.length; i++)
            {
                // Name=Value
                sToken = Tokens[i];
                SubTokens = sToken.split("=");
                if(SubTokens.length == 2)
                {
                    sName = SubTokens[0];
                    sValue = SubTokens[1];
                    // Elements are named by the same name with arguments in the received string
                    Elements = document.getElementsByName(sName);
                    // only display the value if the element exists
                    if(Elements.length > 0)
                    {
                        if(Elements[0].type == "number")
                        {
                            Elements[0].value = parseInt(sValue);
                        }
                        else if(Elements[0].type == "radio")
                        {
                            chosen = parseInt(sValue);
                            if(chosen < Elements.length && chosen >= 0)
                                Elements[chosen].checked = true;
                        }
                        else(Elements[0].type == "text")
                        {
                            Elements[0].value = sValue;
                        }
                    }
                }
            }
        }
    }
    xhttp.open("GET", "getconfig", true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xhttp.setRequestHeader("Connection", "Close");
    xhttp.send();
}

