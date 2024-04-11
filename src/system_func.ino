
static const char serverCACert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFMDCCBBigAwIBAgISBNG02QSCDx60y4HsGSAPu9w7MA0GCSqGSIb3DQEBCwUA
MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD
EwJSMzAeFw0yMjA1MTYwNjUwMzJaFw0yMjA4MTQwNjUwMzFaMBcxFTATBgNVBAMT
DGdyaWItdGVjaC5ydTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOPW
qDVWrwwt9CTQncV3a/KwQdshxna2wvKCaKfwogrzrD+PsIfFJVrV91lz/8IV9/d+
uDSb63sASxvl8oPuxHKaqHxr4nqCbEOIEyyy6pd7AvBXK3aWATnf5N2l8DdBlOMi
arRbag6U3I8quIFX79RkW98jmfE+BKHrIlq4HMIjaLHuh714wLLv5a1qUu/Nz0bC
cE6dxjKAlPczJDL8DM7U73S7F8TVJlKuFLNd2zJhY1jbBEo+n8UKR3wrnoUDrGlb
rqRJa2mSDnpKKd3WkU0fn8Dr8Y1JSZEYNuOW0UGeHH4dmG9EnQFYFZpxFzBBQB8a
kvDb954UL3TVONiDsHcCAwEAAaOCAlkwggJVMA4GA1UdDwEB/wQEAwIFoDAdBgNV
HSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4E
FgQUBX2HRLeU1UrzD/mPmxd+eMKhccQwHwYDVR0jBBgwFoAUFC6zF7dYVsuuUAlA
5h+vnYsUwsYwVQYIKwYBBQUHAQEESTBHMCEGCCsGAQUFBzABhhVodHRwOi8vcjMu
by5sZW5jci5vcmcwIgYIKwYBBQUHMAKGFmh0dHA6Ly9yMy5pLmxlbmNyLm9yZy8w
KQYDVR0RBCIwIIIMZ3JpYi10ZWNoLnJ1ghB3d3cuZ3JpYi10ZWNoLnJ1MEwGA1Ud
IARFMEMwCAYGZ4EMAQIBMDcGCysGAQQBgt8TAQEBMCgwJgYIKwYBBQUHAgEWGmh0
dHA6Ly9jcHMubGV0c2VuY3J5cHQub3JnMIIBBAYKKwYBBAHWeQIEAgSB9QSB8gDw
AHYAQcjKsd8iRkoQxqE6CUKHXk4xixsD6+tLx2jwkGKWBvYAAAGAy9juEQAABAMA
RzBFAiEA9y6HoJ1Wu9W9Xo2hnAlWnvxKaV3YkIEZn7gO9xFjO8oCIDmH4k3mgNCh
+gylONt8hgCz7w6YVJr1IBDrPkq4HPwMAHYARqVV63X6kSAwtaKJafTzfREsQXS+
/Um4havy/HD+bUcAAAGAy9juIAAABAMARzBFAiBVDQAKe5sokZM0gkAezyfhD9RJ
qFrtUS6idpATyAZ+MQIhAKp3Q1XGm5DdYxrxE+1zXafCW6q/rXUA7xO1/r06SviF
MA0GCSqGSIb3DQEBCwUAA4IBAQA/DsuHgVbU//0KezIcctG+z+UM3v0YyZYK7awx
zZolPTiBQhCIAxbPVxBtbRnTkJllZ55fTFTLVoVR6HgH74qi22tBHo3Yp9LRm/M4
i7qXLiH1ZyvPVaFHNhDtn+L4BrLIZqap+nrcn3hV7XEZjcGQaaMogBHH7sw3zugN
wqtotE1YWMARhmil6vM8F1V9RWkq0OV1K0emLSmvXGIuK1T173BSkdJ0/2H7gs0W
ygujuoyEjQyQd2WCZ+eZZw9BN98lsR18gOt9fcQE+nN44+4HjWgBDCiuFwTlvOJ/
xtrpyKpz/BNmYBm2wtuug6nbeqzg6oLHTDqlCpzO/jLjZaZt
-----END CERTIFICATE-----
)EOF";






int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i = 0; i < n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}



void set_channel() {

  int32_t channel;
  channel = getWiFiChannel(WIFI_SSID);

  if (channel >= 1) {
    //Serial.print("Настраиваем основной канал: ");
    //Serial.println(channel);
    wifi_promiscuous_enable(1);
    wifi_set_channel(channel);
    wifi_promiscuous_enable(0);
  }

  else {

    char ssid[35];
    EEPROM.get(3430, ssid);
    channel = getWiFiChannel(ssid);
    if (channel >= 1) {
      //Serial.print("Настраиваем второстепенный канал: ");
      //Serial.println(channel);
      wifi_promiscuous_enable(1);
      wifi_set_channel(channel);
      wifi_promiscuous_enable(0);
    }
  }
}

void update_firm() {

    BearSSL::WiFiClientSecure client;

    BearSSL::X509List x509(serverCACert);
    client.setTrustAnchors(&x509);

    setClock();

    ESPhttpUpdate.rebootOnUpdate(false); // remove automatic update

    t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://www.grib-tech.ru/update/meteo_v" + String(NEXT_FIRMWARE_VERSION) + ".bin");

    switch (ret) {
      case HTTP_UPDATE_FAILED:

        //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        WiFi.disconnect();

        break;
      case HTTP_UPDATE_NO_UPDATES:
        WiFi.disconnect();
        break;
      case HTTP_UPDATE_OK:

        delay(1000);
        ESP.restart();

        break;
    }
}



uint8_t setClock() {

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    yield();
    now = time(nullptr);
  }
  struct tm * timeinfo;
  timeinfo = localtime(&now);

  uint8_t hour = timeinfo->tm_hour;

  return hour;
}



void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);
  
  
  
  renderJPEG(x, y);
}

void renderJPEG(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.readSwappedBytes()) {
    
    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
    {
      tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
    }
    else if ( (mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }



}

void lcd_get_temp(){
  tft.fillScreen(TFT_BLACK);
  byte temp = dht.readTemperature();
  String mes;
  mes += "+";
  mes += temp;
  mes += " C    ";
  drawArrayJpeg(temperature_jpg, sizeof(temperature_jpg), 7, 22); // Draw a jpeg image stored in memory
  tft.drawString(mes, 64, 29, 4);
}

void lcd_get_hum(){
  tft.fillScreen(TFT_BLACK);
  byte hum = dht.readHumidity();
  String mes;
  mes += hum;
  mes += " %    ";
  drawArrayJpeg(humidity_jpg, sizeof(humidity_jpg), 7, 22); // Draw a jpeg image stored in memory at x,y 
  tft.drawString(mes, 70, 29, 4);
}


void lcd_get_co2(){
  tft.fillScreen(TFT_BLACK);
  int co2 = random(300, 5000);
  String mes;
  mes += co2;
  mes += "     ";
   drawArrayJpeg(co2_jpg, sizeof(co2_jpg), 7, 22); // Draw a jpeg image stored in memory
    tft.drawString(mes, 70, 29, 4);
}

void lcd_get_pres(){
  tft.fillScreen(TFT_BLACK);
  int pres = random(600, 900);
  String mes;
  mes += pres;
  mes += " mm     ";
    drawArrayJpeg(pressure_jpg, sizeof(pressure_jpg), 7, 22); // Draw a jpeg image stored in memory
tft.drawString(mes, 55, 29, 4);
}
