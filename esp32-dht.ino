/* 
  ESP32 dht-sensor 
  simulation d'un système de surveillance de température / humidité avec dashboard web et cloud 
*/

#include <WiFi.h> 
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
//wifi 
const char* ssid="Ooredoo 4G_93F21B" ; 
const char* pwd="8E76665E6B" ;  
 
WebServer server(80) ; 
//simulation des variables dht22 
float temperature = 0;
float humidity = 0;
unsigned long lastSensorUpdate = 0;
//code HTML/CSS/JS pour dashboard web 
const char* root = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Dashboard</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@300;400;600&family=Inter:wght@300;500;700&display=swap');
        
        :root {
            --bg-primary: #0f0f0f;
            --bg-card: #1a1a1a;
            --accent: #c8acd8; 
            --accent-glow: 0 0 20px rgba(200, 172, 216, 0.2);
            --text-primary: #ffffff;
            --text-secondary: #888;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Inter', sans-serif;
            background: var(--bg-primary);
            color: var(--text-primary);
            min-height: 100vh;
            padding: 2rem;
        }
        
        .terminal-header {
            background: var(--bg-card);
            padding: 1rem 1.5rem;
            border-radius: 12px 12px 0 0;
            border-bottom: 1px solid #333;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }
        
        .terminal-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
        }
        
        .dot-blue { background: #90d0ef; }
        .dot-green { background: #9bd58a; }
        .dot-pink { background: #b673a9; }
        
        .dashboard {
            max-width: 800px;
            margin: 0 auto;
            background: var(--bg-card);
            border-radius: 0 0 12px 12px;
            overflow: hidden;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
        }
        
        .grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 1.5rem;
            padding: 2rem;
        }
        
        .metric-card {
            background: rgba(255, 255, 255, 0.05);
            padding: 1.5rem;
            border-radius: 8px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            transition: all 0.3s ease;
        }
        
        .metric-card:hover {
            border-color: var(--accent);
            box-shadow: var(--accent-glow);
            transform: translateY(-2px);
        }
        
        .metric-value {
            font-family: 'JetBrains Mono', monospace;
            font-size: 2rem;
            font-weight: 600;
            color: var(--accent);
            margin: 0.5rem 0;
        }
        
        .metric-label {
            font-size: 0.9rem;
            color: var(--text-secondary);
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .status-indicator {
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            padding: 0.5rem 1rem;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 20px;
            font-size: 0.9rem;
        }
        
        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: #666;
            animation: pulse 2s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        .controls {
            grid-column: 1 / -1;
            display: flex;
            gap: 1rem;
            margin-top: 1rem;
        }
        
        .btn {
            flex: 1;
            background: rgba(0, 0, 0, 0.4);
            color: var(--accent);
            border: 1px solid var(--accent);
            padding: 1rem 1.5rem;
            border-radius: 8px;
            font-family: 'JetBrains Mono', monospace;
            font-size: 0.9rem;
            font-weight: 600;
            text-align: center;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        
        .btn:hover {
            background: var(--accent);
            color: var(--bg-primary);
            box-shadow: var(--accent-glow);
        }
        
        .last-update {
            text-align: center;
            padding: 1rem;
            color: var(--text-secondary);
            font-size: 0.8rem;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <div class="terminal-header">
            <div class="terminal-dot dot-blue"></div>
            <div class="terminal-dot dot-green"></div>
            <div class="terminal-dot dot-pink"></div>
            <span style="margin-left: 1rem; font-family: 'JetBrains Mono', monospace; font-size: 0.9rem;">
                mini-dashboard
            </span>
        </div>
        
        <div class="grid">
            <div class="metric-card">
                <div class="metric-label">Temperature</div>
                <div class="metric-value" id="temperature">°C</div>
                <div class="status-indicator">
                    <div class="status-dot" id="temp-dot"></div>
                    <span id="temp-status">Loading...</span>
                </div>
            </div>
            
            <div class="metric-card">
                <div class="metric-label">Humidity</div>
                <div class="metric-value" id="humidity">-- %</div>
                <div class="status-indicator">
                    <div class="status-dot" id="humidity-dot"></div>
                    <span id="humidity-status">Loading...</span>
                </div>
            </div>
            
            <div class="controls">
                <button class="btn" onclick="refreshData()">
                    <span style="font-size: 1.2rem;"></span><br>
                    REFRESH DATA 
                </button>
            </div>
           <div class="last-update">
             Last updated: <span id="last-update">--</span>
             </div> 
        </div>
    </div>

    <script>
        function getStatusColor(temp) {
            if (temp < 18) return { color: '#90d0ef', status: 'Cool' };
            if (temp > 28) return { color: '#ff6b6b', status: 'Hot' };
            return { color: '#9bd58a', status: 'Normal' };
        }
        
        function getHumidityStatus(humidity) {
            if (humidity < 30) return { color: '#ffa726', status: 'Dry' };
            if (humidity > 70) return { color: '#90d0ef', status: 'Humid' };
            return { color: '#9bd58a', status: 'Comfortable' };
        }

        function updateDashboard(data) {
            document.getElementById('temperature').textContent = data.temperature.toFixed(1) + 'C';
            const tempStatus = getStatusColor(data.temperature);
            document.getElementById('temp-dot').style.backgroundColor = tempStatus.color;
            document.getElementById('temp-status').textContent = tempStatus.status;
            
            document.getElementById('humidity').textContent = data.humidity.toFixed(1) + '%';
            const humidityStatus = getHumidityStatus(data.humidity);
            document.getElementById('humidity-dot').style.backgroundColor = humidityStatus.color;
            document.getElementById('humidity-status').textContent = humidityStatus.status;
            
            document.getElementById('last-update').textContent = new Date().toLocaleTimeString();
        }

        function refreshData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    updateDashboard(data);
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('temperature').textContent = 'Error';
                    document.getElementById('humidity').textContent = 'Error';
                });
        }
        
        document.addEventListener('DOMContentLoaded', function() {
            refreshData();
        });
    </script>
</body>
</html>
)=====";
void connectwifi() {  
  WiFi.begin(ssid,pwd) ; 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500) ;  
    Serial.println('.');
  } 
  Serial.println("connected ! "); 
  Serial.println("adresse IP= ") ; 
  Serial.print(WiFi.localIP()) ; 
}

//simulation dh22 : générer des valeurs aléatoires 
 void updateSensorData() { 
  temperature = random (200 , 350) / 10.0 ; 
  humidity = random (300 , 800) / 10.0 ; 
 }
 void handleData() { 
// retourne les données du capteur en JSON 
// appelé quand JS fetch '/data' 
  String json = "{";
  json += "\"temperature\":" + String(temperature, 1);
  json += ",\"humidity\":" + String(humidity, 1);
  json += "}";
  server.send(200, "application/json", json);
} 

void handleNotFound() {
  server.send(404, "text/plain", "Page not found");
}

void handleroot() {  
  Serial.println("Client connected ! "); 
  server.send(200, "text/html", root);
}
void sendbb() { 
   const String  url="http://api.beebotte.com/v1/data/write/esp32_dht" ; 
   HTTPClient http ; 
   http.begin( url ) ; 
   http.addHeader("Content-type", "application/json");
   http.addHeader("X-Auth-Token" , "token_lbc3jvank9jTM3Ca");
   String data ; 
   StaticJsonDocument <256> json ; 
   //crée JSON file 
   json["records"][0]["resource"] = "temperature" ; 
   json["records"][0]["data"] = temperature ; 
   json["records"][1]["resource"] = "humidity" ; 
   json["records"][1]["data"] = humidity ;
   serializeJson (json , data) ; 

   Serial.print("Raw temperature value: "); Serial.println(temperature, 6); 
   Serial.print("Raw humidity value: "); Serial.println(humidity, 6);
   Serial.print("JSON being sent: "); Serial.println(data);
   Serial.print("JSON length: "); Serial.println(data.length());

   int code = http.POST(data) ; 
   if ( code>0) { 
    Serial.println(code) ; 
   }else { 
    Serial.println("erreur") ; 
   } 
   http.end() ; 
}
 

void setup() { 
  Serial.begin(9600);
  delay(1000) ;   
  connectwifi() ; 
  updateSensorData();
  server.on("/" , handleroot) ; 
  server.on("/data", handleData); //API JSON 
  server.onNotFound(handleNotFound);
  server.begin() ; // demarrer serveur web 
  
}
void loop() { 
  server.handleClient(); // gérer les requetes web 

    //mettre a jour les données toutes les 2 secondes 
  if (millis() - lastSensorUpdate > 2000) {
    updateSensorData();
    lastSensorUpdate = millis();
    
    Serial.print(" Sensor Data - Temp: ");
    Serial.print(temperature, 1);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity, 1);
    Serial.println("%");
    //envoyer les données au cloud toutes les 30 secondes 
     static unsigned long lastBeebotteSend = 0;
    if (millis() - lastBeebotteSend > 30000) {
      sendbb();
      lastBeebotteSend = millis();
    }
  
  }
  
  delay(100); 
  
}

 
