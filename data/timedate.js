Number.prototype.pad = function(size) {
    var s = String(this);
    while (s.length < (size || 2)) {s = "0" + s;}
    return s;
}

function advanceTimedate(t) {
   var timedate=t.split("-");
   var date=timedate[0].split(".");
   var time=timedate[1].split(":");
   var Snew = (Number(time[2]) + 1) % 60;
   var Mnew = (Number(time[1]) + Math.floor((Number(time[2]) + 1 ) /60 ))%60;
   var Hnew = (Number(time[0]) + Math.floor((Number(time[1])) /60 ))%24;
   return Number(date[0]).pad()+"."+Number(date[1]).pad()+"."+Number(date[2]).pad()+"-"+Hnew.pad()+":"+Mnew.pad()+":"+Snew.pad();

}
