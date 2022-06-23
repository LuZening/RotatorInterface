function point(x, y)
{
    this.x = x;
    this.y = y;
    this.get_norm=function()
    {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    };
    this.div=function(n)
    {
        this.x = this.x / n;
        this.y = this.y / n;
    };
    this.mult=function(n)
    {
        this.x = this.x * n;
        this.y = this.y * n;
    };
    this.sub=function(p)
    {
        this.x = this.x - p.x;
        this.y = this.y - p.y;
    };
    this.plus=function(p)
    {
        this.x = this.x + p.x;
        this.y = this.y + p.y;
    }
    this.get_sub=function(p)
    {
        return new point(this.x-p.x, this.y-p.y);
    };
    this.unify=function()
    {
        this.div(this.get_norm());
    };
};