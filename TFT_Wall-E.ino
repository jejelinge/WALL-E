/*
 An example digital clock using a TFT LCD screen to show the time.
 Demonstrates use of the font printing routines. (Time updates but date does not.)

 It uses the time of compile/upload to set the time
 For a more accurate clock, it would be better to use the RTClib library.
 But this is just a demo...

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################

 Based on clock sketch by Gilchrist 6/2/2014 1.0

A few colour codes:

code	color
0x0000	Black
0xFFFF	White
0xBDF7	Light Gray
0x7BEF	Dark Gray
0xF800	Red
0xFFE0	Yellow
0xFBE0	Orange
0x79E0	Brown
0x7E0	Green
0x7FF	Cyan
0x1F	Blue
0xF81F	Pink

 */
#include "FS.h"
#include "TFT_eSPI.h"
#include "Bitmap.h"
#include "DFRobotDFPlayerMini.h"
#include "RTClib.h"

//================= USEFUL VARIABLES ==========================
uint16_t notification_volume = 25;            //Set volume value. From 0 to 30
#define CALIBRATION_FILE "/calibrationData11"  // This is the file name used to store the touch coordinate - Change the name to start a new calibration.

//=============================================================

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false
#define FPSerial Serial1

const byte RXD2 = 16;  // Connects to module's TX => 16
const byte TXD2 = 17;  // Connects to module's RX => 17

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

RTC_DS3231 rtc;

byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;
int mode = 0;
int valide = 1;
int toggle_lamp = 0;
int toggle_alarm = 0;
int Play_finished = 0;
uint16_t calibrationData[5];
uint8_t calDataOK = 0;
int hour = 0;
int minute = 0;
int play = 0;
int alarm_set = 0;
int font_val = 7;
uint16_t x = 0, y = 0;
uint32_t targetTime = 0;
int hh;
int mm;
int ss;
int stop = 1;
int alarm_stop = 0;
int alarm_active = 0;
int valide_clock=1;
int valide_volume = 1;

void setup(void) {

  pinMode(33, OUTPUT);
  pinMode(35, INPUT);

  Serial.begin(115200);
  Serial.println("let's go");

  tft.init();
  tft.setRotation(1);

  // call screen calibration
  touch_calibrate();

  //Fill the screen with black
  tft.fillScreen(TFT_BLACK);

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  tft.setTextSize(1);
  tft.drawBitmap(0, 0, epd_bitmap_Interface_2, 320, 240, 0xFDE0);
  tft.setTextColor(0xFDE0, TFT_BLACK);

  targetTime = millis() + 1000;
delay(1000);
  FPSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(FPSerial, /*isACK = */ true, /*doReset = */ true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0);
    }
  }

Serial.println(F("DFPlayer Mini online."));
myDFPlayer.volume(notification_volume);
myDFPlayer.setTimeOut(500); // Définit un temps de time out sur la communication série à 500 ms
Serial.println(myDFPlayer.readFileCounts()); 
Serial.println(myDFPlayer.readCurrentFileNumber()); 

  delay(1000);
  myDFPlayer.playMp3Folder(1);  //Play the first mp3
}

void loop() {

  DateTime now = rtc.now();

  hh = now.hour();
  mm = now.minute();
  ss = now.second();

  // tft.fillRect(10, 200, 50, 40, TFT_BLACK);
  // Serial.println(analogRead(35));
  // tft.setTextColor(0xFFFF);
  // tft.drawNumber(analogRead(35), 20, 210, 2);
if((currentMonth*30 + monthDay) >= 121 && (currentMonth*30 + monthDay) < 331){
timeClient.setTimeOffset(utcOffsetInSeconds*UTC);} // Change daylight saving time - Summer
else {timeClient.setTimeOffset((utcOffsetInSeconds*UTC) - 3600);} // Change daylight saving time - Winter

  if(alarm_active == 1){ alarm_rings();} 

  set_time();

  // Setup the alarm
  if (tft.getTouch(&x, &y)) {
    if ((x > 20) && (x < 165)) {
      if ((y > 150) && (y < 210)) {
        set_alarm();
        delay(200);
      }
    }
  }

  // Change the volume
  if (tft.getTouch(&x, &y)) {
    if ((x > 190) && (x < 320)) {
      if ((y > 30) && (y < 240)) {
        myDFPlayer.playMp3Folder(random(5, 9));
        delay(200);
        volume_notification();
      }
    }
  }

  // Switch on the bulb or stop the alarm
  if (tft.getTouch(&x, &y) && toggle_lamp == 0) {
    if ((x > 100) && (x < 170)) {
      if ((y > 50) && (y < 120)) {
        digitalWrite(33, HIGH);
        toggle_lamp = 1;
        delay(500);
        myDFPlayer.playMp3Folder(2);
        delay(200);
      }
    }
  }

  // Switch off the bulb or stop the alarm
  if (tft.getTouch(&x, &y) && toggle_lamp == 1) {
    if ((x > 100) && (x < 170)) {
      if ((y > 50) && (y < 120)) {
        digitalWrite(33, LOW);
        toggle_lamp = 0;
        delay(150);
      }
    }
  }
}

void volume_notification()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(0xFDE0);
  tft.drawString("Volume", 122, 10, 4);
  tft.drawRect(270, 190, 50, 50, 0xFDE0);
  tft.drawString("OK", 276, 205, 4);
  tft.fillTriangle(140, 80, 160, 40, 180, 80, 0xFDE0);
  tft.fillTriangle(140, 190, 160, 230, 180, 190, 0xFDE0);
  
  while (valide_volume) {

    tft.drawNumber(notification_volume, 125, 110, font_val);              // Draw volume value between 0 and 30

    if (tft.getTouch(&x, &y)) {
      if ((x > 270) && (x < 320)) {
        if ((y > 190) && (y < 240)) {
          valide_volume = 0;
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 120) && (x < 160)) {
        if ((y > 30) && (y < 100)) {
          notification_volume = notification_volume + 1;
          if (notification_volume > 30) { notification_volume = 30; }
          Serial.print("notification_volume : ");
          Serial.print(notification_volume);
          tft.fillRect(20, 100, 300, 80, TFT_BLACK);
          delay(50);
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 120) && (x < 160)) {
        if ((y > 180) && (y < 240)) {
          notification_volume = notification_volume - 1;
          if (notification_volume < 0) { notification_volume = 0; }
          Serial.print("notification_volume : ");
          Serial.print(notification_volume);
          tft.fillRect(20, 100, 300, 80, TFT_BLACK);
          delay(50);
        }
      }
    }
  }

  myDFPlayer.volume(notification_volume);
  valide_volume = 1;
  myDFPlayer.playMp3Folder(random(5, 9));
  delay(100);
  tft.fillScreen(TFT_BLACK);
  tft.drawBitmap(0, 0, epd_bitmap_Interface_2, 320, 240, 0xFDE0);
  alarm_set = 1;
  set_time();

}

void alarm_rings() {
  if (hh == hour && mm == minute && play == 0) {
    myDFPlayer.playMp3Folder(random(3, 5));
    delay(200);
    while (stop) {
      // Switch on the bulb
      if (tft.getTouch(&x, &y)) {
        if ((x > 100) && (x < 170)) {
          if ((y > 50) && (y < 120)) {
            myDFPlayer.stop();
            delay(200);
            stop = 0;
          }
        }
      }
    }
    play = 1;
    stop = 1;
    Serial.println("boucle alarm rings");
    Serial.println(play);
  }
}

void battery_tester() {
  if (analogRead(35) > 2250) {
    for (int gap = 0; gap <= 135; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2250 && analogRead(35) > 2225) {
    tft.fillRect(195, 52, 113, 8, TFT_BLACK);

    for (int gap = 0; gap <= 120; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2225 && analogRead(35) > 2200) {

    for (int gap = 0; gap <= 15; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }

    for (int gap = 0; gap <= 105; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2200 && analogRead(35) > 2175) {

    for (int gap = 0; gap <= 30; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }

    for (int gap = 0; gap <= 90; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2175 && analogRead(35) > 2150) {

    for (int gap = 0; gap <= 45; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }

    for (int gap = 0; gap <= 75; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2150 && analogRead(35) > 2125) {

    for (int gap = 0; gap <= 60; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }

    for (int gap = 0; gap <= 60; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2125 && analogRead(35) > 2100) {

    for (int gap = 0; gap <= 75; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }

    for (int gap = 0; gap <= 30; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2100 && analogRead(35) > 2075) {

    for (int gap = 0; gap <= 90; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }

    for (int gap = 0; gap <= 15; gap = gap + 15) {
      tft.fillRect(195, 187 - gap, 113, 8, 0xFDE0);
    }
  }

  if (analogRead(35) < 2075 && analogRead(35) > 2000) {

    for (int gap = 0; gap <= 105; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }

    tft.fillRect(195, 187, 113, 8, 0xFDE0);
  }

  if (analogRead(35) < 2000) {
    for (int gap = 0; gap <= 135; gap = gap + 15) {
      tft.fillRect(195, 52 + gap, 113, 8, TFT_BLACK);
    }
  }
}

void set_alarm() {
  int triangle_w = 60;
  int triangle_h = 40;

  int triangle1_x = 50;
  int triangle1_y = 80;

  int triangle2_x = 200;
  int triangle2_y = 80;

  int triangle3_x = 50;
  int triangle3_y = 170;

  int triangle4_x = 200;
  int triangle4_y = 170;

  Serial.println("SET ALARM");
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(0xFDE0);
  tft.drawString("Alarm setup", 95, 10, 4);
  tft.drawRect(270, 190, 50, 50, 0xFDE0);
  tft.drawString("OK", 276, 205, 4);
  tft.drawRect(117, 200, 80, 30, 0xFDE0);

  tft.fillTriangle(triangle1_x, triangle1_y, triangle1_x + (triangle_w / 2), triangle1_y - triangle_h, triangle1_x + triangle_w, triangle1_y, 0xFDE0);
  tft.fillTriangle(triangle2_x, triangle2_y, triangle2_x + (triangle_w / 2), triangle2_y - triangle_h, triangle2_x + triangle_w, triangle2_y, 0xFDE0);
  tft.fillTriangle(triangle3_x, triangle3_y, triangle3_x + (triangle_w / 2), triangle3_y + triangle_h, triangle3_x + triangle_w, triangle3_y, 0xFDE0);
  tft.fillTriangle(triangle4_x, triangle4_y, triangle4_x + (triangle_w / 2), triangle4_y + triangle_h, triangle4_x + triangle_w, triangle4_y, 0xFDE0);

  tft.fillTriangle(280, 100, 310, 120, 280, 140, 0xFDE0);

  while (valide) {
    // Update digital time
    int x_alarm = 90;
    int y_alarm = 100;  // Top left corner ot clock text, about half way down
    // Draw hours and minutes
    if (hour < 10) x_alarm += tft.drawChar('0', x_alarm, y_alarm, font_val);  // Add hours leading zero for 24 hr clock
    x_alarm += tft.drawNumber(hour, x_alarm, y_alarm, font_val);              // Draw hours
    xcolon = x_alarm;                                                         // Save colon coord for later to flash on/off later
    x_alarm += tft.drawChar(':', x_alarm, y_alarm, font_val);
    if (minute < 10) x_alarm += tft.drawChar('0', x_alarm, y_alarm, font_val);  // Add minutes leading zero
    x_alarm += tft.drawNumber(minute, x_alarm, y_alarm, font_val);              // Draw minutes

    if (tft.getTouch(&x, &y)) {
      if ((x > 270) && (x < 320)) {
        if ((y > 190) && (y < 240)) {
          valide = 0;
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 40) && (x < 110)) {
        if ((y > 30) && (y < 120)) {
          hour = hour + 1;
          Serial.print("hour : ");
          Serial.print(hour);
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);

          if (hour > 23) { hour = 0; }
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 40) && (x < 110)) {
        if ((y > 140) && (y < 240)) {
          hour = hour - 1;
          Serial.print("hour : ");
          Serial.print(hour);
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);
          if (hour < 0) { hour = 23; }
        }
      }
    }
    if (tft.getTouch(&x, &y)) {
      if ((x > 180) && (x < 260)) {
        if ((y > 30) && (y < 120)) {
          minute = minute + 1;
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);
          if (minute > 59) { minute = 0; }
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 180) && (x < 240)) {
        if ((y > 140) && (y < 220)) {
          minute = minute - 1;
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);
          if (minute < 0) { minute = 59; }
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 110) && (x < 210)) {
        if ((y > 170) && (y < 240)) {
          if (alarm_active == 0) {
            alarm_active = 1;
           tft.fillRect(118, 201, 39, 28, TFT_BLACK);
           tft.fillRect(159, 201, 39, 28, 0xFDE0);
            delay(200);
          }
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 110) && (x < 210)) {
        if ((y > 170) && (y < 240)) {
          if (alarm_active == 1) {
            alarm_active = 0;
           tft.fillRect(159, 201, 37, 28, TFT_BLACK);
           tft.fillRect(118, 201, 39, 28, 0x31A6);
            delay(200);
          }
        }
      }
    }

    if (alarm_active == 1) {
      tft.fillRect(118, 201, 39, 28, TFT_BLACK);
      tft.fillRect(159, 201, 39, 28, 0xFDE0);
    }

    if (alarm_active == 0) {
      tft.fillRect(159, 201, 37, 28, TFT_BLACK);
      tft.fillRect(118, 201, 39, 28, 0x31A6);
    }

  if (tft.getTouch(&x, &y)) {
      if ((x > 270) && (x < 320)) {
        if ((y > 90) && (y < 140)) {
          clock_setup();
          valide = 0;
        }
      }
    }

  }

  Serial.println("Set alarm out");
  tft.fillScreen(TFT_BLACK);
  tft.drawBitmap(0, 0, epd_bitmap_Interface_2, 320, 240, 0xFDE0);
  valide = 1;
  alarm_set = 1;
  set_time();
}

void set_time() {

  DateTime now = rtc.now();
  hh = now.hour();
  mm = now.minute();
  ss = now.second();

  if (targetTime < millis()) {
    // Set next update for 1 second later
    targetTime = millis() + 1000;

    // Adjust the time values by adding 1 second
    ss++;             // Advance second
    if (ss == 60) {   // Check for roll-over
      ss = 0;         // Reset seconds to zero
      omm = mm;       // Save last minute time for display update
      mm++;           // Advance minute
      if (mm > 59) {  // Check for roll-over
        mm = 0;
        hh++;           // Advance hour
        if (hh > 23) {  // Check for 24hr roll-over (could roll-over on 13)
          hh = 0;       // 0 for 24 hour clock, set to 1 for 12 hour clock
        }
      }
    }

    // Update digital time
    int xpos = 20;
    int ypos = 150;  // Top left corner ot clock text, about half way down
    int ysecs = ypos + 24;
    Serial.println("Entree set time");
    Serial.println(play);
    if (omm != mm || alarm_set == 1) {  // Redraw hours and minutes time every minute
      omm = mm;
      battery_tester();
      if (alarm_set == 1) { tft.fillRect(10, 150, 160, 50, TFT_BLACK); }
      if (play == 1) { delay(2000); }
      play = 0;
      alarm_set = 0;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, font_val);  // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, font_val);              // Draw hours
      xcolon = xpos;                                                 // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos, font_val);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, font_val);  // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, font_val);              // Draw minutes
      xsecs = xpos;                                                  // Sae seconds 'x' position for later display updates
    }
    if (oss != ss) {  // Redraw seconds time every second
      oss = ss;
      xpos = xsecs;

      if (ss % 2) {                                 // Flash the colons on/off
        tft.setTextColor(TFT_BLACK, TFT_BLACK);     // Set colour to grey to dim colon
        tft.drawChar(':', xcolon, ypos, font_val);  // Hour:minute colon
        //xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
        tft.setTextColor(0xFDE0, TFT_BLACK);  // Set colour back to yellow

      } else {
        tft.drawChar(':', xcolon, ypos, font_val);  // Hour:minute colon
        // xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      }

      //Draw seconds
      // if (ss < 10) xpos += tft.drawChar('0', xpos, ysecs, 6); // Add leading zero
      // tft.drawNumber(ss, xpos, ysecs, 6);                     // Draw seconds
    }
  }
}

void clock_setup(){

  int triangle_w = 60;
  int triangle_h = 40;

  int triangle1_x = 50;
  int triangle1_y = 80;

  int triangle2_x = 200;
  int triangle2_y = 80;

  int triangle3_x = 50;
  int triangle3_y = 170;

  int triangle4_x = 200;
  int triangle4_y = 170;

  DateTime now = rtc.now();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(0xFDE0);
  tft.drawString("Clock setup", 95, 10, 4);
  tft.drawRect(270, 190, 50, 50, 0xFDE0);
  tft.drawString("OK", 276, 205, 4);

  tft.fillTriangle(triangle1_x, triangle1_y, triangle1_x + (triangle_w / 2), triangle1_y - triangle_h, triangle1_x + triangle_w, triangle1_y, 0xFDE0);
  tft.fillTriangle(triangle2_x, triangle2_y, triangle2_x + (triangle_w / 2), triangle2_y - triangle_h, triangle2_x + triangle_w, triangle2_y, 0xFDE0);
  tft.fillTriangle(triangle3_x, triangle3_y, triangle3_x + (triangle_w / 2), triangle3_y + triangle_h, triangle3_x + triangle_w, triangle3_y, 0xFDE0);
  tft.fillTriangle(triangle4_x, triangle4_y, triangle4_x + (triangle_w / 2), triangle4_y + triangle_h, triangle4_x + triangle_w, triangle4_y, 0xFDE0);

  while (valide_clock) {
    // Update digital time
    int x_alarm = 90;
    int y_alarm = 100;  // Top left corner ot clock text, about half way down
    // Draw hours and minutes
    if (hh < 10) x_alarm += tft.drawChar('0', x_alarm, y_alarm, font_val);  // Add hours leading zero for 24 hr clock
    x_alarm += tft.drawNumber(hh, x_alarm, y_alarm, font_val);              // Draw hours
    xcolon = x_alarm;                                                         // Save colon coord for later to flash on/off later
    x_alarm += tft.drawChar(':', x_alarm, y_alarm, font_val);
    if (mm < 10) x_alarm += tft.drawChar('0', x_alarm, y_alarm, font_val);  // Add minutes leading zero
    x_alarm += tft.drawNumber(mm, x_alarm, y_alarm, font_val);              // Draw minutes

    if (tft.getTouch(&x, &y)) {
      if ((x > 270) && (x < 320)) {
        if ((y > 190) && (y < 240)) {
          valide_clock = 0;
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 40) && (x < 110)) {
        if ((y > 30) && (y < 120)) {
          hh = hh + 1;
          if (hh > 23) { hh = 0; }
          Serial.print("hh : ");
          Serial.print(hh);
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), hh, mm, 0));
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);
 
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 40) && (x < 110)) {
        if ((y > 140) && (y < 240)) {
          hh = hh - 1;
          if (hh < 0) { hh = 23; }
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), hh, mm, 0));
          Serial.print("hh : ");
          Serial.print(hh);
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);
        }
      }
    }
    if (tft.getTouch(&x, &y)) {
      if ((x > 180) && (x < 260)) {
        if ((y > 30) && (y < 120)) {
          mm = mm + 1;
          if (mm > 59) { mm = 0; }
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), hh, mm, 0));
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);
        }
      }
    }

    if (tft.getTouch(&x, &y)) {
      if ((x > 180) && (x < 240)) {
        if ((y > 140) && (y < 220)) {
          mm = mm - 1;
          if (mm < 0) { mm = 59; }
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), hh, mm, 0));
          tft.fillRect(20, 90, 300, 60, TFT_BLACK);
          delay(50);
        }
      }
    }


  }

  valide_clock = 1;
}

void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    } else {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char *p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      Play_finished = 1;
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void waitMilliseconds(uint16_t msWait) {
  uint32_t start = millis();

  while ((millis() - start) < msWait) {
    // calling mp3.loop() periodically allows for notifications
    // to be handled without interrupts
    delay(1);
  }
}