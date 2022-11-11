/******
 * Draw the coverage fan of the antenna, following the mouse arrow
 * Author: Zening Lu
 * Date: Apr. 28, 2018
 */

fan_span = 60;
pos_fan = [0, 0]; // popsition where the coverage fan points at 0-360
var is_mouseon = false;
var pos_mouse = new point(-1,-1);
var deg_task = NaN;

function d2rad(deg)
{
    return deg/180*Math.PI;
}

var canv2 = document.createElement("canvas")
canv2.width = X_SIZE;
canv2.height = Y_SIZE;
function azu_draw()
{
    if (!is_projected)
        return;
    var canv = document.getElementById("azu_map");
    var ctx = canv.getContext("2d");
    var ctx2 = canv2.getContext("2d")
    var loc_origin = new point(X_SIZE/2, Y_SIZE/2);
    // draw projection
    ctx2.reset();
    ctx2.putImageData(data_azu, 0, 0);
    // draw coverage fan
    if(pos_fan[0] != NaN)
    {
        fan_l = d2rad(pos_fan[0] - fan_span/2 - 90);
        fan_c = d2rad(pos_fan[0] - 90);
        fan_r = d2rad(pos_fan[0] + fan_span/2 - 90);
        var loc_fanend_l = new point(
            X_SIZE/2 * Math.cos(fan_l), X_SIZE/2 * Math.sin(fan_l)
        );
        var loc_fanend_r = new point(
            X_SIZE/2 * Math.cos(fan_r), X_SIZE/2 * Math.sin(fan_r)
        );
        var loc_cline = new point(
            X_SIZE/2 * Math.cos(fan_c), X_SIZE/2 * Math.sin(fan_c)
        )
        ctx2.fillStyle = "rgba(199,255,236,0.5)";
        ctx2.beginPath();
        ctx2.translate(loc_origin.x, loc_origin.y);
        ctx2.moveTo(0,0);
        ctx2.arc(0, 0, X_SIZE/2,fan_l, fan_r);
        ctx2.closePath();
        ctx2.fill();
        // central line
        ctx2.beginPath();
        ctx2.moveTo(0, 0);
        ctx2.lineTo(loc_cline.x, loc_cline.y);
        ctx2.closePath();
        ctx2.strokeStyle = "rgba(114,109,209,0.5)";
        ctx2.stroke();
    }
    // draw target line
    if(isAzuTargetSet)
    {
        var nTo = nAzuTargetTo % 360;
        if(nTo < 0) nTo += 360;
        radLine = d2rad(nTo - 90);
        var loc_tline = new point(
            X_SIZE/2 * Math.cos(radLine), X_SIZE/2 * Math.sin(radLine)
        );
        // draw line
        ctx2.beginPath();
        ctx2.moveTo(0, 0);
        ctx2.lineTo(loc_tline.x, loc_tline.y);
        ctx2.closePath();
        ctx2.strokeStyle="rgba(255,109,209,0.3)";
        ctx2.stroke();
        // draw flag
        loc_flag = loc_tline;
        loc_flag.mult(0.65);
        ctx2.drawImage(imgFlag, loc_flag.x,loc_flag.y,X_SIZE / 20, Y_SIZE/20);

    }
    // draw mouse drag line
    if(is_mouseon)
    {
        var point_to = new point(pos_mouse.x, pos_mouse.y);
        point_to.sub(loc_origin);
        point_to.unify();
        deg_task = deg(Math.atan2(point_to.y, point_to.x)) + 90;
        while (deg_task < 0) deg_task += 360; 
        point_to.mult(X_SIZE/2);
        ctx2.strokeStyle="rgba(114,109,209,0.5)";
        ctx2.beginPath()
        ctx2.moveTo(0,0);
        ctx2.lineTo(point_to.x, point_to.y);
        ctx2.closePath()
        ctx2.stroke();
    }
    ctx.drawImage(canv2, 0, 0, X_SIZE, Y_SIZE);
}

function azu_onmousemove()
{
    var canv = document.getElementById("azu_map");
    var ctx = canv.getContext("2d");
    var canv_bound = canv.getBoundingClientRect();
    pos_mouse.x = (event.clientX - canv_bound.left)*(canv.width / canv_bound.width);
    pos_mouse.y = (event.clientY - canv_bound.top)*(canv.height / canv_bound.height);
    is_mouseon = true;

}

function azu_onmouseleave()
{
    is_mouseon = false;
}

function azu_onmouseup()
{
    if(!is_mouseon) return;
    if(deg_task != NaN)
    {
        create_task_target(1);
        send_task();
        azu_mark_target(deg_task);
    }
}
setInterval(function(){azu_draw();}, 30);

imgFlag = new Image();
imgFlag.src = "flag.png";
var isAzuTargetSet = false;
var nAzuTargetTo = 0;
function azu_mark_target(to)
{
    isAzuTargetSet = true;
    nAzuTargetTo = to;
}
function azu_remove_mark_target()
{
    isAzuTargetSet = false;
    nAzuTargetTo = 0;
}