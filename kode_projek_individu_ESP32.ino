#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Kos Bu Tatik";
const char* password = "tatik656565";

WebServer server(80);

// Definisi Pin
const int pinIN3 = 21;
const int pinIN4 = 19;
const int SoilSensor = 34;
const int batasKering = 2500; 

// Status Mode & Motor
bool modeOtomatis = true;
bool statusMotor = false;
int nilaiSensor = 0;

void nyalakanMotor() {
  digitalWrite(pinIN3, HIGH);
  digitalWrite(pinIN4, LOW);
  statusMotor = true;
}

void matikanMotor() {
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, LOW);
  statusMotor = false;
}

// 1. Halaman HTML Utama + JavaScript AJAX
void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<link rel=\"icon\" href=\"data:,\">";
  html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  html += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
  html += ".button2 {background-color: #f44336;} .button3 {background-color: #008CBA;} p {font-size: 24px;}</style>";
  
  // SCRIPT AJAX untuk update data secara real-time (tiap 1 detik)
  html += "<script>";
  html += "setInterval(function() {";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      var data = this.responseText.split(',');"; // Memisahkan data berdasarkan koma
  html += "      document.getElementById('valSensor').innerHTML = data[0];";
  html += "      document.getElementById('valStatus').innerHTML = data[1];";
  html += "      document.getElementById('valMode').innerHTML = data[2];";
  html += "    }";
  html += "  };";
  html += "  xhttp.open('GET', '/readSensor', true);";
  html += "  xhttp.send();";
  html += "}, 1000);"; // 1000ms = 1 detik
  html += "</script>";
  html += "</head>";
  
  html += "<body><h1>Penyiram Tanaman ESP32 (AJAX)</h1>";
  
  // Kontainer teks yang akan diubah oleh JavaScript AJAX
  html += "<p>Nilai Sensor: <strong id='valSensor'>" + String(nilaiSensor) + "</strong></p>";
  
  html += "<p>Status Pompa: <span id='valStatus'>";
  if (statusMotor) html += "<strong style='color:green;'>MENYALA (ON)</strong>";
  else html += "<strong style='color:red;'>MATI (OFF)</strong>";
  html += "</span></p>";

  html += "<p>Mode: <strong id='valMode'>" + String(modeOtomatis ? "OTOMATIS" : "MANUAL") + "</strong></p>";
  
  // Tombol Kontrol
  html += "<p><a href=\"/manual\"><button class=\"button button3\">Ubah ke Manual</button></a> ";
  html += "<a href=\"/otomatis\"><button class=\"button button3\">Ubah ke Otomatis</button></a></p>";
  
  html += "<p><a href=\"/motorOn\"><button class=\"button\">Nyalakan Pompa</button></a> ";
  html += "<a href=\"/motorOff\"><button class=\"button button2\">Matikan Pompa</button></a></p>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// 2. Rute Baru: Hanya mengirim data mentah untuk AJAX
void handleXML() {
  String strStatus = statusMotor ? "<strong style='color:green;'>MENYALA (ON)</strong>" : "<strong style='color:red;'>MATI (OFF)</strong>";
  String strMode = modeOtomatis ? "OTOMATIS" : "MANUAL";
  
  // Format data: NilaiSensor,StatusMotor,ModeOtomatis
  String dataMentah = String(nilaiSensor) + "," + strStatus + "," + strMode;
  
  server.send(200, "text/plain", dataMentah);
}

// Router Aksi
void handleOtomatis() { modeOtomatis = true; server.sendHeader("Location", "/"); server.send(303); }
void handleManual()   { modeOtomatis = false; server.sendHeader("Location", "/"); server.send(303); }
void handleMotorOn()  { if(!modeOtomatis) nyalakanMotor(); server.sendHeader("Location", "/"); server.send(303); }
void handleMotorOff() { if(!modeOtomatis) matikanMotor(); server.sendHeader("Location", "/"); server.send(303); }

void setup() {
  Serial.begin(115200);
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);
  
  matikanMotor(); 

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/readSensor", handleXML); // Rute penyuplai data AJAX
  server.on("/otomatis", handleOtomatis);
  server.on("/manual", handleManual);
  server.on("/motorOn", handleMotorOn);
  server.on("/motorOff", handleMotorOff);
  server.begin();
}

void loop() {
  server.handleClient(); 

  nilaiSensor = analogRead(SoilSensor);

  if (modeOtomatis) {
    if (nilaiSensor > batasKering) {
      if (!statusMotor) nyalakanMotor();
    } else {
      if (statusMotor) matikanMotor();
    }
  }
  
  delay(50); // Delay kecil agar pembacaan loop responsif
}