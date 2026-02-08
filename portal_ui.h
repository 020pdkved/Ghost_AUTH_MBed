#ifndef PORTAL_UI_H
#define PORTAL_UI_H

#include <Arduino.h>

const char PROGMEM index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>VED_OS // NEXUS</title>
<style>
  :root { --neon: #0f0; --bg: #000; --panel: rgba(0,15,0,0.95); --err: #f00; }
  body { margin:0; overflow:hidden; background:var(--bg); color:var(--neon); font-family:'Courier New', monospace; display:flex; align-items:center; justify-content:center; height:100vh; }
  
  /* Matrix Background */
  canvas { position:absolute; top:0; left:0; z-index:-1; opacity:0.25; }
  
  /* Glass Panel */
  .card { 
    position:relative; z-index:10; width:350px; padding:30px; 
    background:var(--panel); border:1px solid var(--neon); 
    box-shadow:0 0 30px rgba(0,255,0,0.2); 
    backdrop-filter:blur(5px); transition:all 0.3s ease;
  }
  
  h2 { margin-top:0; border-bottom:1px solid #333; padding-bottom:15px; letter-spacing:4px; font-size:18px; text-transform:uppercase; }
  
  /* Cyber Inputs */
  .input-box { position:relative; margin-bottom:20px; }
  input { 
    width:100%; padding:15px; background:rgba(0,0,0,0.8); 
    border:1px solid #333; color:var(--neon); text-align:center; 
    font-family:inherit; font-size:16px; letter-spacing:2px; box-sizing:border-box;
    transition:0.3s;
  }
  input:focus { border-color:var(--neon); outline:none; box-shadow:0 0 15px rgba(0,255,0,0.1); }
  input:disabled { opacity:0.5; border-color:#222; cursor:not-allowed; }

  /* Ghost Mode Classes */
  .ghost { color:transparent !important; caret-color:var(--neon); text-shadow:0 0 0 transparent; }
  .unlocked { border-color:var(--neon) !important; color:var(--neon) !important; box-shadow:0 0 15px var(--neon) !important; }

  /* Action Button */
  button { 
    width:100%; padding:15px; background:var(--neon); color:#000; 
    border:none; font-weight:900; letter-spacing:1px; cursor:pointer; 
    text-transform:uppercase; transition:0.2s; 
  }
  button:hover { box-shadow:0 0 20px var(--neon); transform:scale(0.98); }

  /* Utilities */
  .hidden { display:none !important; }
  .status { height:20px; font-size:11px; margin-top:10px; text-align:center; color:var(--err); text-transform:uppercase; }
  .link { margin-top:20px; font-size:10px; text-align:center; cursor:pointer; opacity:0.7; text-decoration:underline; }
  .link:hover { opacity:1; color:var(--neon); }

</style>
</head>
<body>

<canvas id="c"></canvas>

<div id="v1" class="card">
  <h2>SYSTEM ACCESS</h2>
  <div class="input-box"><input type="text" id="u" placeholder="IDENTITY" autocomplete="off"></div>
  <div class="input-box"><input type="text" id="p" class="ghost" placeholder="AWAITING HANDSHAKE..." disabled autocomplete="off"></div>
  <div id="msg" class="status"></div>
  <div class="link" onclick="swap('v2')">[ INITIATE NEW PROTOCOL ]</div>
</div>

<div id="v2" class="card hidden">
  <h2>NEW PROTOCOL</h2>
  <div class="input-box"><input type="text" id="nu" placeholder="SET IDENTITY"></div>
  <div class="input-box"><input type="text" id="nk" placeholder="SET HANDSHAKE"></div>
  <div class="input-box"><input type="password" id="np" placeholder="SET PASSWORD"></div>
  <button onclick="reg()">WRITE TO MEMORY</button>
  <div id="smsg" class="status"></div>
  <div class="link" onclick="swap('v1')">[ << RETURN ]</div>
</div>

<div id="v3" class="card hidden" style="border-color:#0ff; box-shadow:0 0 30px #0ff; color:#0ff;">
  <h2 style="border-color:#0ff;">ACCESS GRANTED</h2>
  <div style="text-align:left; line-height:2; font-size:13px; border-bottom:1px solid #005555; padding-bottom:15px; margin-bottom:15px;">
    STATUS: <span style="float:right;">ONLINE</span><br>
    LATENCY: <span style="float:right;">3ms</span><br>
    ENCRYPTION: <span style="float:right;">AES-256</span><br>
  </div>
  <div style="text-align:left; font-size:12px; opacity:0.8;">
    USER: VEDANG PHADKE<br>
    PRN: 1032210634<br>
    BATCH: A1 / CSF<br>
  </div>
  <button style="background:#0ff; margin-top:25px;" onclick="location.reload()">TERMINATE SESSION</button>
</div>

<script>
// --- MATRIX RAIN ENGINE ---
const C=document.getElementById('c'), X=C.getContext('2d');
C.width=window.innerWidth; C.height=window.innerHeight;
const S='01ABCDEFGHIJKLMNOPQRSTUVWXYZ', A=Array(Math.floor(C.width/14)).fill(1);
function draw(){
  X.fillStyle='rgba(0,0,0,0.05)'; X.fillRect(0,0,C.width,C.height);
  X.fillStyle='#0f0'; X.font='14px monospace';
  A.forEach((y,i)=>{
    X.fillText(S[Math.floor(Math.random()*S.length)], i*14, y*14);
    if(y*14>C.height && Math.random()>0.975) A[i]=0; A[i]++;
  }); requestAnimationFrame(draw);
} draw();

// --- LOGIC CORE ---
let db={u:"",k:"",p:""}, ok=false;

function swap(id){ document.querySelectorAll('.card').forEach(e=>e.classList.add('hidden')); document.getElementById(id).classList.remove('hidden'); if(id=='v1')rst(); }

function rst(){ 
  document.getElementById('u').value=""; 
  let p=document.getElementById('p'); 
  p.value=""; p.disabled=true; p.type="text"; p.className="ghost"; p.placeholder="AWAITING HANDSHAKE..."; 
  ok=false; 
  document.getElementById('msg').innerText="";
}

function reg(){ 
  let u=document.getElementById('nu').value, k=document.getElementById('nk').value, p=document.getElementById('np').value;
  if(!u||!k||!p){ document.getElementById('smsg').innerText="NULL DATA DETECTED"; return; }
  db={u:u,k:k,p:p}; 
  document.getElementById('smsg').style.color="#0f0";
  document.getElementById('smsg').innerText="PROTOCOL WRITTEN";
  setTimeout(()=>swap('v1'),1500);
}

// LOGIN FLOW
document.getElementById('u').onkeydown = e => { 
  if(e.key=='Enter'){ 
    if(document.getElementById('u').value.trim()!=""){
      document.getElementById('p').disabled=false; 
      document.getElementById('p').focus(); 
    } else {
      document.getElementById('msg').innerText="IDENTITY REQUIRED";
    }
  } 
};

// THE RITUAL
document.getElementById('p').onkeydown = e => {
  // If already unlocked, Enter submits password
  if(ok && e.key=='Enter'){
    if(document.getElementById('u').value==db.u && document.getElementById('p').value==db.p) swap('v3');
    else document.getElementById('msg').innerText="ACCESS DENIED";
  }
  // If locked, Backspace checks handshake
  if(!ok && e.key=='Backspace'){
    e.preventDefault();
    if(document.getElementById('p').value==db.k && db.k!=""){ 
      ok=true; 
      let p=document.getElementById('p'); 
      p.value=""; p.type="password"; p.className="unlocked"; p.placeholder="ENTER PASSWORD //"; 
    } else {
      let p=document.getElementById('p'); p.value="";
      document.getElementById('msg').innerText="HANDSHAKE FAILED";
      setTimeout(()=>document.getElementById('msg').innerText="",1000);
    }
  }
};
</script>
</body>
</html>
)rawliteral";

#endif