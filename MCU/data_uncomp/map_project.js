/******
 * Azumithal Equidistant map projector
 * Reproject Azumithal Equidistant (aka Great Circle) map 
 * from plain Cylindrical World Map
 * Author: Zening Lu
 * Date: Apr. 27, 2018
 */
function rad(deg)
{
    return deg / 180 * Math.PI;
}
function deg(rad)
{
    return rad / Math.PI * 180;
}
const X_SIZE = 512;
const Y_SIZE = 512;
long_0 = 121;// degree
lat_0 = 32; //degree
//adjusted
long_0_rad = rad(long_0-149);
lat_0_rad = rad(-lat_0);
// prepare bitmap data of the cylindrical map 
canvas_t = document.createElement('canvas');
img_cymap = new Image();
img_azu = document.getElementById("azu_map");
ctx_azu = img_azu.getContext("2d");
data_azu = ctx_azu.createImageData(X_SIZE, Y_SIZE);
img_cymap.src="world_map_cylindrical_tiny.png";
is_projected = false; // indicate if the projection has been prepared
img_cymap.onload = function() // draw the picture on the canvas
{  
    // img_cymap.style="display:none"
    // draw the new projection
    canvas_t.width=this.width;
    canvas_t.height=this.height;
    ctx_cymap = canvas_t.getContext('2d');
    ctx_cymap.drawImage(img_cymap,0,0,this.width,this.height);
    img_azu.width = X_SIZE;
    img_azu.height = Y_SIZE;
    var i = 0;
    for (y=Math.floor(-Y_SIZE/2);y<Math.floor(Y_SIZE/2);++y)
    {
        for(x=Math.floor(-X_SIZE/2);x<Math.floor(X_SIZE/2);++x)
        {
            // back trace to the cylindrical map
            var x_u = x / X_SIZE * Math.PI * 2;
            var y_u = y/ Y_SIZE * Math.PI * 2;
            var c = Math.sqrt(x_u*x_u+y_u*y_u);
            
            var lat_1_rad =  Math.asin(Math.cos(c) * Math.sin(lat_0_rad) + (y_u * Math.sin(c) * Math.cos(lat_0_rad))/c)
            if(Math.abs(lat_0-90) < 0.01)
            {
                var long_1_rad = Math.atan(-x_u/y_u);
            }
            else if(Math.abs(lat_0+90)<0.01)
            {
                var long_1_rad = Math.atan(x_u/y_u);
            }
            else
            {
                var long_1_rad = Math.atan2((x_u * Math.sin(c)), (c*Math.cos(lat_0_rad)*Math.cos(c)-y_u*Math.sin(lat_0_rad)*Math.sin(c)));
            }
            long_1_rad = (long_1_rad + long_0_rad);
            if(long_1_rad < -Math.PI) {long_1_rad = long_1_rad + Math.PI * 2;}
            // find the coordinate in the original map picture
            x_0 = Math.round((long_1_rad / (2*Math.PI)+0.5) * img_cymap.width);
            y_0 = Math.round((lat_1_rad / Math.PI + 0.5) * img_cymap.height);
            //y_0 = Math.round(Math.tan(lat_1_rad) + img_cymap.height/2 );
            try {
                pix_data = ctx_cymap.getImageData(x_0,y_0,1,1).data;
                // copy data
                if(c<Math.PI)
                {
                    data_azu.data[i*4] = pix_data[0];
                    data_azu.data[i*4+1] = pix_data[1];
                    data_azu.data[i*4+2] = pix_data[2];
                    data_azu.data[i*4+3] = pix_data[3];
                }
            } catch (error) {
                
            }
            i=i+1;
        }
    }

    ctx_azu.putImageData(data_azu,0,0);
    is_projected = true;
    // mark the central point
    ctx_azu.fillStyle = "#000000";
    ctx_azu.beginPath();
    ctx_azu.arc(X_SIZE/2, Y_SIZE/2, 2.5,0, Math.PI*2, true);
    ctx_azu.closePath();
    ctx_azu.fill();
    document.getElementById("map_loading").remove();
}

