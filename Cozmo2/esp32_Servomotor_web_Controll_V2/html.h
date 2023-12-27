char index2_html[] PROGMEM = R"=====(
<!DOCTYPE html>
      <head>
      <meta charset="utf-8"> <title>Wifi PPM (V1.1)</title>
      <meta name="viewport" content="width=device-width, user-scalable=no">
  
      <style>
    
    .container {
    font-family: arial;
    font-size:24px;
    margin: 25px;
    wind: 400px;
    height: 300px;
    outline: dashed 1px plack;
    position: relative;
    }
  .child{
    width: 50px;
    height: 50px;
    background-color: green;
    position: absolute;

    }
  
      .switch, .voltage {
        position: relative;
        display: block;
        margin-left: auto;
        margin-right: auto;
        width: 34px;
        height: 34px;
      }

      .voltage {
        position: relative;
        display: block;
        margin-left: auto;
        margin-right: auto;
        width: 54px;
        height: 34px;
      }

      .switch input {display:none;}
  
      .slider {
        position: absolute;
        cursor: pointer;
        border-radius: 34px;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #F00;
      }
  
      input:checked + .slider {
        background-color: #0F0;
      }
    
    
    /* DivTable.com */
.divTable{
  display: table;
  width: 33%;
}
.divTableRow {
  display: table-row;
}
.divTableHeading {
  background-color: #EEE;
  display: table-header-group;
}
.divTableCell, .divTableHead {
  border: 1px solid #999999;
  display: table-cell;
  padding: 3px 10px;
}
.divTableHeading {
  background-color: #EEE;
  display: table-header-group;
  font-weight: bold;
}
.divTableFoot {
  background-color: #EEE;
  display: table-footer-group;
  font-weight: bold;
}
.divTableBody {
  display: table-row-group;
}
  
      </style>
      </head>
    <html>
      <body>




<form>
<input type='range' min='0' max='180' value='90' id='servoSlider'>
<p>Servo Angle: <span id='servoValue'>90</span></p>
  

<div class="divTable" style="width: 100%;display: flex;border: 1px solid #000;flex-direction: column;align-items: center;gap: 20px">
  <div class="divTableBody">
    <div class="divTableRow">
      <div class="divTableCell"> </div>
      <div class="divTableCell"><p>Button 1: <span id='button1State'></span></p> <button onclick='toggleButton1()'>Toggle Button 1</button> </div>
      <div class="divTableCell"> </div>
    </div>
    <div class="divTableRow">
      <div class="divTableCell"> <p>Button 3: <span id='button3State'></span></p> <button onclick='toggleButton3()'>Toggle Button 3</button></div>
      <div class="divTableCell"> </div>
      <div class="divTableCell"> <p>Button 4: <span id='button4State'></span></p> <button onclick='toggleButton4()'>Toggle Button 4</button></div>
    </div>
    <div class="divTableRow">
      <div class="divTableCell"> </div>
      <div class="divTableCell"><p>Button 2: <span id='button2State'></span></p> <button onclick='toggleButton2()'>Toggle Button 2</button> </div>
      <div class="divTableCell"> </div>
    </div>
    <img id="stream" src="http://192.168.4.2:81/stream" crossorigin="anonymous">
  </div>
</div>

</form></body>
<script>
  document.querySelector('#servoSlider').addEventListener('input', function() {
  document.querySelector('#servoValue').textContent = this.value;
  fetch('servo?angle=' + this.value);
  });

  function toggleButton1() {
    fetch('/button1');
  }
  function toggleButton2() {
    fetch('/button2');
  }
  function toggleButton3() {
    fetch('/button3');
  }
  function toggleButton4() {
    fetch('/button4');
  }
  
 </script>
 </html>
)=====";
