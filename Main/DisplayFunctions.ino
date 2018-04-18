boolean initOLED() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(WHITE, BLACK);
    display.display();
    return true;
}

void splashScreen(int thyme) {
  display.clearDisplay();
  display.drawBitmap(0, 0, rambot_logo, 128, 32, 1);
  display.display();
  delay(time);
}

void line1(String s) {
  display.setCursor(0, 0);
  display.print(s);
  display.display();
}

void line2(String s) {
  display.setCursor(0, 10);
  display.print(s);
  display.display();
}

void line3(String s) {
  display.setCursor(0, 20);
  display.print(s);
  display.display();
}

