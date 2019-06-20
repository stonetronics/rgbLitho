#include <WiFi.h>
#include <WebServer.h>

/* Put your SSID & Password */
const char* ssid = "RGB Lithophane Lantern";  // Enter SSID here
const char* password = "!5uper5afePa55w0rd&";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,0,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

uint8_t rPin = 12;
uint8_t gPin = 13;
uint8_t bPin = 14;
uint8_t light = 0;

int color = 0x8E00FF;

void setup() {
  light = 1;
  Serial.begin(115200);
  
  
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);

  ledcAttachPin(rPin, 0);
  ledcAttachPin(gPin, 1);
  ledcAttachPin(bPin, 2);  

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/on", handle_on);
  server.on("/off", handle_off);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  if(light)
  {
    //pwm the rgb
    ledcWrite(0, (color>>16)&0xFF );
    ledcWrite(1, (color>>8)&0xFF );
    ledcWrite(2, (color)&0xFF );
    
  } else {
    //turn the rgb off
    digitalWrite(rPin, LOW);
    digitalWrite(gPin, LOW);
    digitalWrite(bPin, LOW);
  }
}

void handle_OnConnect() {
  light = 1;
  color = handleSubmit();
  server.send(200, "text/html", SendHTML(light, color)); 
}

void handle_on() {
  light = 1;
  Serial.println("Light ON");
  color = handleSubmit();
  server.send(200, "text/html", SendHTML(light, color)); 
}

void handle_off() {
  light = 0;
  Serial.println("Light OFF");
  color = handleSubmit();
  server.send(200, "text/html", SendHTML(light, color)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


int handleSubmit(){
  int l_color;
  int noArgs = server.args();
  for (int i = 0; i < noArgs; i++)
  {
    Serial.print("Arg no ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(server.argName(i));
    Serial.print(":");
    Serial.println(server.arg(i));
  }
  
  if (!server.hasArg("color"))
    return color;

  unsigned char colorString[7] = {};
  server.arg("color").substring(1,8).getBytes(colorString, 7);

  l_color = (int) strtol((const char*)colorString, NULL, 16);
  Serial.print("colorString: ");
  Serial.println((const char*)colorString);
  Serial.print("color changed! is now: ");
  Serial.println(l_color);
  Serial.print("color as HEX: 0x");
  Serial.println(String(l_color,HEX));

  return l_color;
}

String stringHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "%%.%dX", precision);

     sprintf(tmp, format, num);
     return String(tmp);
}


String SendHTML(uint8_t light, uint32_t color){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>RGB Lithophane Lantern</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>RGB Lithophane Lantern</h1>\n";
  ptr +="<h3>Color:</h3>\n";

  ptr +="<div>\n";
  ptr +="<form id=colorPickerForm>\n";
  ptr +="  <input type=\"color\" id=\"color\" name=\"color\" value=\"#" + stringHex(color,6) + "\" onchange=\"this.form.submit()\">\n";        
  ptr +="</form\n>";
  ptr +="</div>\n";
  
  if(light)
  {ptr +="<p>light on</p><a class=\"button button-off\" href=\"/off\">off</a>\n";}
  else
  {ptr +="<p>light off</p><a class=\"button button-on\" href=\"/on\">on</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
