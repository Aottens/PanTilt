const char index_html[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang='en'>
<meta charset='utf-8'>
<title>PanTilt</title>
<style>
body{font-family:sans-serif;margin:20px;max-width:600px}
label{display:block;margin-top:10px}
button{margin:5px}
#plot{width:100%;height:200px;border:1px solid #ccc;margin-top:10px}
</style>
<div>
<h2>Pan/Tilt Head</h2>
<label>Pan Min<input id=pmin type=number></label>
<label>Pan Max<input id=pmax type=number></label>
<label>Tilt Min<input id=tmin type=number></label>
<label>Tilt Max<input id=tmax type=number></label>
<label>Dwell<input id=dwell type=number step=0.1></label>
<button onclick="send('/api/center','POST')">Center</button>
<button onclick="send('/api/sequence/start','POST')">Seq Start</button>
<button onclick="send('/api/sequence/stop','POST')">Seq Stop</button>
<button onclick="store(0)">Store A</button>
<button onclick="recall(0)">Recall A</button>
<button onclick="store(1)">Store B</button>
<button onclick="recall(1)">Recall B</button>
<button onclick="store(2)">Store X</button>
<button onclick="recall(2)">Recall X</button>
<button onclick="store(3)">Store Y</button>
<button onclick="recall(3)">Recall Y</button>
<button onclick="send('/api/wipe','POST')">Wipe</button>
<canvas id=plot></canvas>
</div>
<script>
let evt=new EventSource('/events');
evnt=evt;evt.onmessage=e=>{let d=JSON.parse(e.data);plot(d.pan,d.tilt)};
function send(url,method,data){fetch(url,{method,body:data});}
function store(n){send('/api/keyframe/store/'+n,'POST');}
function recall(n){send('/api/keyframe/recall/'+n,'POST');}
fetch('/api/params').then(r=>r.json()).then(d=>{pmin.value=d.panMin;pmax.value=d.panMax;tmin.value=d.tiltMin;tmax.value=d.tiltMax;dwell.value=d.dwell;});
[pmin,pmax,tmin,tmax,dwell].forEach(el=>el.onchange=()=>{let fd=new FormData();fd.append(el.id,el.value);send('/api/params','POST',fd);});
let ctx=plot.getContext('2d');let w=plot.width,h=plot.height;function plot(p,t){ctx.clearRect(0,0,w,h);ctx.fillRect(w/2+p/180*w/2,h/2-t/90*h/2,5,5);}
</script>
)HTML";
