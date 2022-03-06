/******
 * Draw the coverage fan of the antenna, following the mouse arrow
 * Author: Zening Lu
 * Date: Apr. 28, 2018
 */

fan_span = 60;
pos_fan = 100; // popsition where the coverage fan points at 0-360
var is_mouseon = false;
var pos_mouse = new point(-1,-1);
var deg_task = NaN;
function d2rad(deg)
{
    return deg/180*Math.PI;
}


function azu_draw()
{
    var canv = document.getElementById("azu_map");
    var ctx = canv.getContext("2d");
    var canv_bound = canv.getBoundingClientRect();
    var loc_origin = new point(X_SIZE/2, Y_SIZE/2);
    // draw projection
    ctx.putImageData(data_azu, 0, 0);
    // draw coverage fan
    if(pos_fan != NaN)
    {
        fan_l = d2rad(pos_fan - fan_span/2 - 90);
        fan_c = d2rad(pos_fan - 90);
        fan_r = d2rad(pos_fan + fan_span/2 - 90);
        var loc_fanend_l = new point(
            X_SIZE/2 * Math.cos(fan_l), X_SIZE/2 * Math.sin(fan_l)
        );
        var loc_fanend_r = new point(
            X_SIZE/2 * Math.cos(fan_r), X_SIZE/2 * Math.sin(fan_r)
        );
        var loc_cline = new point(
            X_SIZE/2 * Math.cos(fan_c), X_SIZE/2 * Math.sin(fan_c)
        )
        ctx.fillStyle = "rgba(199,255,236,0.5)";
        ctx.beginPath();
        ctx.translate(loc_origin.x, loc_origin.y);
        ctx.moveTo(0,0);
        ctx.arc(0,0,X_SIZE/2,fan_l, fan_r);
        ctx.closePath();
        ctx.fill();
        // central line
        ctx.beginPath();
        ctx.moveTo(0,0);
        ctx.lineTo(loc_cline.x, loc_cline.y);
        ctx.strokeStyle = "rgba(114,109,209,0.3)";
        ctx.stroke();
        ctx.translate(-loc_origin.x, -loc_origin.y);
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
        ctx.strokeStyle="rgba(114,109,209,0.5)";
        ctx.beginPath()
        ctx.translate(loc_origin.x, loc_origin.y);
        ctx.moveTo(0,0);
        ctx.lineTo(point_to.x, point_to.y);
        ctx.stroke();
        ctx.translate(-loc_origin.x, -loc_origin.y);
    }
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
        create_task_target();
        send_task();
    }
}
setInterval(function(){azu_draw();}, 20);
