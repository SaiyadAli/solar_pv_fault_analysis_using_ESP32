#include <WiFi.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WebServer server(80);

const char* ssid = "Ishani";
const char* password = "1234";
int PANEL_PIN_34 = 34; // Pin for panel 1
int PANEL_PIN_35 = 35; // Pin for panel 2
#define MAX_FAULTS 5

String faultStatuses_34[MAX_FAULTS];
String timestamps_34[MAX_FAULTS];
int currentFaultIndex_34 = 0;

String faultStatuses_35[MAX_FAULTS];
String timestamps_35[MAX_FAULTS];
int currentFaultIndex_35 = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  pinMode(PANEL_PIN_34, INPUT_PULLUP); // Configure pin 34 as input with internal pull-up resistor
  pinMode(PANEL_PIN_35, INPUT_PULLUP); // Configure pin 35 as input with internal pull-up resistor

  Serial.begin(9600);

  connectToWifi();

  timeClient.setTimeOffset(19800); // Set time offset to adjust to local time zone
  timeClient.begin(); // Initialize NTP client

  beginServer();
}

void loop() {
  server.handleClient();
  delay(5000); // Wait for 5 seconds
  updateFaultStatus(); // Check the fault status after the delay
  timeClient.update(); // Update NTP client
}

void connectToWifi() {
  WiFi.enableSTA(true);
  delay(2000);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void beginServer() {
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  server.send(200, "text/html", getPage());
}

void updateFaultStatus() {
  int panelState_34 = digitalRead(PANEL_PIN_34);
  if (panelState_34 == HIGH) {
    addFaultStatus_34("Faulty");
  } else {
    addFaultStatus_34("Normal");
  }

  int panelState_35 = digitalRead(PANEL_PIN_35);
  if (panelState_35 == HIGH) {
    addFaultStatus_35("Faulty");
  } else {
    addFaultStatus_35("Normal");
  }
}

void addFaultStatus_34(String status) {
  faultStatuses_34[currentFaultIndex_34] = status;
  timestamps_34[currentFaultIndex_34] = timeClient.getFormattedTime();
  currentFaultIndex_34 = (currentFaultIndex_34 + 1) % MAX_FAULTS; // Move to the next index, wrapping around if necessary
}

void addFaultStatus_35(String status) {
  faultStatuses_35[currentFaultIndex_35] = status;
  timestamps_35[currentFaultIndex_35] = timeClient.getFormattedTime();
  currentFaultIndex_35 = (currentFaultIndex_35 + 1) % MAX_FAULTS; // Move to the next index, wrapping around if necessary
}

String getPage() {
  String page = "<html lang=en-EN><head><meta http-equiv='refresh' content='5'/>";
  page += "<title>Solar Panel Fault Status</title>";
  page += "<style> body { background-color: #f9f9f9; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: #333; }";
  page += "h1 { color: #007bff; margin-bottom: 20px; }";
  page += "table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }";
  page += "th, td { text-align: left; padding: 12px 15px; border-bottom: 1px solid #ddd; }";
  page += "th { background-color: #f2f2f2; }";
  page += "tr:nth-child(even) { background-color: #f2f2f2; }";
  page += "</style>";
  page += "</head><body><div style='max-width: 800px; margin: 0 auto; padding: 20px;'>";
  page += "<h1>Solar Panel Fault Status</h1>";
  page += "<table>";
  page += "<tr><th>Time</th><th>Solar Panel 1</th><th>Solar Panel 2</th></tr>";
  for (int i = 0; i < MAX_FAULTS; i++) {
    page += "<tr>";
    String timestamp = timestamps_34[(currentFaultIndex_34 + i) % MAX_FAULTS]; // Use timestamp from panel 1
    page += "<td>" + timestamp + "</td>";
    page += "<td>" + faultStatuses_34[(currentFaultIndex_34 + i) % MAX_FAULTS] + "</td>";
    page += "<td>" + faultStatuses_35[(currentFaultIndex_35 + i) % MAX_FAULTS] + "</td>";
    page += "</tr>";
  }
  page += "</table>";
  page += "</div></body></html>";
  return page;
}




