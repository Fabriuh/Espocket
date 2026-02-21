// =========== INCLUIR LIBRERIAS ===========
#include <Wire.h>
#include <U8g2lib.h>
#include <RTClib.h>
#include <EEPROM.h>
// ================== DISPLAY ==================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(
  U8G2_R0,
  U8X8_PIN_NONE
);
// ============= RTC ===========
RTC_DS3231 rtc;
// =========== DEFINICIONES =========
#define SDA_PIN 8
#define SCL_PIN 9
#define BUZZER_PIN 3
#define BTN_LEFT   21
#define BTN_CENTER 20
#define BTN_RIGHT  10
#define WIN_SCORE 15
#define MAX_FISH    3
#define MAX_FOOD    3
#define MAX_BUBBLES 6
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define STAR_COUNT 250
#define SCREEN_W 128
#define SCREEN_H 64
#define FONT_W 6
#define FONT_H 9
#define MATRIX_COLS (SCREEN_W / FONT_W)
#define SCREENSAVER_TIMEOUT 60000UL
#define MAX_ALARMS 4
#define EEPROM_ALARM_START 0
#define ALARM_CHECK_INTERVAL 500  // ms entre verificaciones
#define ALARM_BLINK_INTERVAL 500   // ms para parpadeo
#define ALARM_BUZZER_FREQ 2000     // Frecuencia de la alarma
#define ALARM_BUZZER_DURATION 200  // Duración de cada pitido
#define ALARM_SNOOZE_TIME 300000   // 5 minutos en ms
#define HOLD_DELAY 300
#define EEPROM_MAGIC_ADDR 100  
#define EEPROM_MAGIC_NUMBER 0xAA
#define EEPROM_CONFIG_ADDR 200  // Dirección para guardar config (después de alarmas)
// ============= VARIABLES SALVAPANTALLAS 1 y 2===========
bool screensaverEnabled = true;
struct Star {
  float x;
  float y;
  float z;
};
Star stars[STAR_COUNT];
float warpAngle = 0.0;
struct MatrixCol {
  int y;
  int speed;
  int length;
};
MatrixCol matrix[MATRIX_COLS];
uint32_t lastInteractionMillis = 0;
enum ScreenSaverID {
  SAVER_MATRIX = 0,
  SAVER_STARS = 1,
  SAVER_COUNT   // SIEMPRE el último
};
ScreenSaverID activeSaver = SAVER_STARS;
bool screensaverTransition = false;
int transitionOffset = 0;
uint8_t transitionBuffer[SCREEN_WIDTH * (SCREEN_HEIGHT / 8)];
uint32_t lastTransitionFrame = 0;
const uint8_t TRANSITION_SPEED = 20; // ms por frame
// ================= VARIABLES PONG =================
const int PADDLE_H = 14;
const int PADDLE_W = 2;
const int BALL_SIZE = 2;

int playerY, aiY;
int ballX, ballY;
int ballVX, ballVY;

int playerScore = 0;
int aiScore = 0;

unsigned long lastPongFrame = 0;
enum PongState {
  PONG_PLAY,
  PONG_WIN,
  PONG_LOSE
}; // Estados para pantalla (ganaste, perdiste, jugando)
PongState pongState = PONG_PLAY;
// ================ PECERA ===============
struct Fish {
  int16_t x, y;
  int8_t size;
  int8_t speed;
  bool direction;
  int8_t eaten;
  int8_t targetFood;   // DEBE ser SIGNED
};
struct Food {
  int x, y;
  bool active;
}; // Sujeto comida
struct Bubble {
  int x, y;
  int speed;
}; // Sujeto burbuja
Fish fishes[MAX_FISH];
Food foods[MAX_FOOD];
Bubble bubbles[MAX_BUBBLES];
// ============== PIBBLE =================
static const unsigned char image_Pibbles_wash_my_belly_drawing_1_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x00,0x00,0xc0,0xd2,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x03,0x00,0x00,0x00,0x00,0x08,0x00,0x0c,0x00,0x00,0x00,0x00,0x04,0x00,0x18,0x00,0x00,0x00,0x00,0x02,0x00,0x30,0x00,0x00,0x00,0x00,0x02,0x00,0x10,0x00,0x00,0x00,0x00,0x01,0x00,0x20,0x00,0x00,0x00,0x40,0x01,0x00,0x50,0x07,0x00,0x00,0x50,0x01,0x00,0x40,0x06,0x00,0x00,0x08,0x00,0x1b,0x9c,0x04,0x00,0x00,0xc4,0x47,0x0a,0xbe,0x04,0x00,0x00,0xcc,0x97,0x50,0xbc,0x01,0x00,0x00,0xbc,0x87,0x81,0x88,0x01,0x00,0x00,0x30,0x00,0x05,0x21,0x01,0x00,0x00,0x20,0x00,0x00,0x00,0x03,0x00,0x00,0x20,0x04,0x02,0x02,0x02,0x00,0x00,0x20,0x00,0x02,0x00,0x02,0x00,0x00,0x20,0x02,0x06,0x00,0x06,0x00,0x00,0x00,0x00,0x0f,0x08,0x02,0x00,0x00,0x10,0x81,0x10,0x08,0x02,0x00,0x00,0x10,0x41,0x20,0x08,0x02,0x00,0x00,0x10,0x31,0xc0,0x06,0x01,0x00,0x00,0x30,0x0e,0x00,0x81,0x11,0x00,0x00,0x70,0x20,0x40,0x40,0x2b,0x00,0x40,0x97,0x80,0x08,0x08,0x66,0x00,0x60,0x0d,0x00,0x02,0x00,0x80,0x00,0x20,0x08,0x00,0x00,0x00,0xe0,0x00,0x40,0x00,0x00,0x00,0x00,0x40,0x00,0x20,0x00,0x00,0x00,0x00,0x50,0x00,0xe0,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x01,0x00,0x00,0x00,0x18,0x00,0x00,0x01,0x00,0x00,0x00,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x00,0x00,0x30,0x00,0x80,0x00,0x00,0x00,0x00,0x30,0x00,0x80,0x00,0x00,0x00,0x00,0x30,0x00,0x80,0x00,0x00,0x00,0x00,0x30,0x00,0x80,0x00,0x00,0x00,0x00,0x30,0x00,0x80,0x01,0x00,0x00,0x00,0x30,0x00,0x80,0x01,0x00,0x00,0x00,0x10,0x00,0x00,0x03,0x00,0x00,0x00,0x18,0x00,0x00,0x02,0x00,0x00,0x00,0x18,0x00,0x00,0x02,0x00,0x00,0x00,0x0c,0x00,0x00,0x01,0x00,0x00,0x00,0x0c,0x00,0x00,0x01,0x00,0x00,0x00,0x2c,0x00,0x80,0x00,0x00,0x00,0x40,0x28,0x00,0x40,0xe0,0x00,0x00,0x70,0x30,0x00,0xc0,0x31,0x0a,0x00,0xce,0x10,0x00,0x00,0x15,0xb0,0xea,0x80,0x34,0x00,0x00,0x1d,0x00,0x08,0x80,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char image_hand_stop_bits[] = {0x30,0x00,0xd8,0x00,0x54,0x01,0x54,0x01,0x56,0x01,0x55,0x01,0x55,0x01,0x55,0x19,0x55,0x15,0x05,0x13,0x01,0x09,0x01,0x04,0x01,0x04,0x02,0x02,0x02,0x01,0x84,0x01};
unsigned long animTimer = 0;
uint8_t estadopibble;
bool pibbleentered = false;
void drawwash_my_belly(void) {
    display.clearBuffer();
    display.setFontMode(1);
    display.setBitmapMode(1);
    display.setFont(u8g2_font_5x7_tr);
    display.drawStr(58, 20, "yo soy pibble");

    display.drawStr(71, 32, "wash my");

    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(72, 45, "Belly");

    display.drawXBM(5, 0, 53, 60, image_Pibbles_wash_my_belly_drawing_1_bits);

    display.sendBuffer();
}
void drawyaaay(void) {
    display.clearBuffer();
    display.setFontMode(1);
    display.setBitmapMode(1);
    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(65, 25, "yayyyy!!");

    display.setDrawColor(2);
    display.drawXBM(26, 39, 13, 16, image_hand_stop_bits);

    display.setDrawColor(1);
    display.drawEllipse(38, 41, 1, 1);

    display.drawEllipse(23, 48, 1, 1);

    display.drawEllipse(20, 42, 1, 1);

    display.drawXBM(5, 1, 53, 60, image_Pibbles_wash_my_belly_drawing_1_bits);

    display.sendBuffer();
}
void drawclean_my_belly(void) {
    display.clearBuffer();
    display.setFontMode(1);
    display.setBitmapMode(1);
    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(65, 25, "Clean my");

    display.drawEllipse(38, 41, 1, 1);

    display.drawEllipse(23, 48, 1, 1);

    display.drawEllipse(20, 42, 1, 1);

    display.drawStr(72, 40, "belly");

    display.drawXBM(5, 1, 53, 60, image_Pibbles_wash_my_belly_drawing_1_bits);

    display.sendBuffer();
}
void drawcleean_my_belly(void) {
    display.clearBuffer();
    display.setFontMode(1);
    display.setBitmapMode(1);
    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(65, 25, "Clean my");

    display.setDrawColor(2);
    display.drawXBM(26, 39, 13, 16, image_hand_stop_bits);

    display.setDrawColor(1);
    display.drawXBM(5, 1, 53, 60, image_Pibbles_wash_my_belly_drawing_1_bits);

    display.drawStr(72, 40, "belly =)");

    display.sendBuffer();
}
// ================= ALARMA ===================
enum AlarmState {
  ALARM_IDLE,
  ALARM_SETUP_HOUR_TENS,
  ALARM_SETUP_HOUR_UNITS,
  ALARM_SETUP_MINUTE_TENS,
  ALARM_SETUP_MINUTE_UNITS,
  ALARM_TRIGGERED
};
struct Alarm {
  uint8_t hour;
  uint8_t minute;
  bool enabled;
  bool snoozed;
  unsigned long snoozeUntil;
};
struct AlarmSlot {
  uint8_t index;
  uint8_t hourTens;
  uint8_t hourUnits;
  uint8_t minuteTens;
  uint8_t minuteUnits;
  AlarmState state;
  unsigned long lastBlink;
  bool blinkState;
  int animOffset;
  unsigned long lastAnim;
  unsigned long lastLeftPress; 
  unsigned long lastRightPress;
};
Alarm alarms[MAX_ALARMS];
AlarmSlot currentAlarmSlot;
uint8_t selectedAlarmSlot = 0;
unsigned long lastAlarmCheck = 0;
unsigned long alarmTriggeredTime = 0;
bool bothButtonsPressedForAlarm = false;
unsigned long bothButtonsPressTime = 0;
const unsigned long bothButtonsHoldTime = 50; // ms para detectar presión simultánea
// ========== FUNCIONES DE EEPROM ==========
void saveAlarmsToEEPROM() {
  int addr = EEPROM_ALARM_START;
  for (int i = 0; i < MAX_ALARMS; i++) {
    EEPROM.put(addr, alarms[i]);
    addr += sizeof(Alarm);
  }
  // Guardar número mágico
  EEPROM.write(100, 0xAA);
  
  // IMPORTANTE: Hacer commit para ESP32
  EEPROM.commit();
  
  delay(50);
}

void loadAlarmsFromEEPROM() {
  // Leer número mágico
  uint8_t magic = EEPROM.read(100);
  
  if (magic == 0xAA) {
    int addr = EEPROM_ALARM_START;
    for (int i = 0; i < MAX_ALARMS; i++) {
      EEPROM.get(addr, alarms[i]);
      addr += sizeof(Alarm);
      
      // Validar datos
      if (alarms[i].hour > 23 || alarms[i].minute > 59) {
        alarms[i].hour = 0;
        alarms[i].minute = 0;
        alarms[i].enabled = false;
      }
    }
  } else {
    // Inicializar todo
    for (int i = 0; i < MAX_ALARMS; i++) {
      alarms[i].hour = 0;
      alarms[i].minute = 0;
      alarms[i].enabled = false;
      alarms[i].snoozed = false;
      alarms[i].snoozeUntil = 0;
    }
    saveAlarmsToEEPROM(); // Guardar estado inicial
  }
}
// ================= CONFIGURACION =================
enum Difficulty {
  DIFF_EASY,
  DIFF_NORMAL,
  DIFF_HARD
};
unsigned long lastBubbleSound = 0;
bool soundEnabled = true;
Difficulty gameDifficulty = DIFF_NORMAL;
// Qué opción estoy editando
uint8_t configIndex = 0;
enum ConfigType {
  CONFIG_SOUND,
  CONFIG_DIFFICULTY,
  CONFIG_SCREENSAVER
};
struct ConfigItem {
  const char* name;
  ConfigType type;
};
ConfigItem configItems[] = {
  { "Sonido", CONFIG_SOUND },
  { "Dificultad", CONFIG_DIFFICULTY },
  { "Salvapantallas", CONFIG_SCREENSAVER}
};
const uint8_t CONFIG_COUNT = sizeof(configItems) / sizeof(configItems[0]);
// ================== MENU =====================
const char* menuItems[] = {
  "Reloj",
  "Pecera",
  "Pong",
  "Configuracion",
  "Alarma",
  "Salvapantallas",
  "Pibble"
}; // Agregar acá para expandir la cantidad
// ===== VARIABLES MENU ====
unsigned long lastCenterPress = 0;
const unsigned long debounceTime = 180; // Debounce para boton del medio
const uint8_t MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);
enum ScreenState { // Pantallas disponibles
  SCREEN_MENU,
  SCREEN_CLOCK,
  SCREEN_ALARM_MENU,
  SCREEN_ALARM_SETUP,
  SCREEN_ALARM_ACTIVE,
  SCREEN_PONG,
  SCREEN_CONFIG,
  SCREEN_PECERA,
  SCREEN_SCREENSAVER,
  SCREEN_PIBBLE
};
ScreenState currentScreen = SCREEN_CLOCK;
int currentIndex = 0;
// animación slide
bool isSliding = false;
int slideDir = 0;
float slideProgress = 0.0f;
unsigned long lastAnim = 0;
// ================== HELPERS ==================
inline int idx(int i) {
  if (i < 0) return i + MENU_COUNT;
  if (i >= MENU_COUNT) return i - MENU_COUNT;
  return i;
}
// flotación SOLO centro
int centerFloat() {
  return sin(millis() * 0.006f) * 3;
}
// ================== CONTROL ==================
void menuNext() {
  if (isSliding) return;
  slideDir = +1;
  isSliding = true;
  slideProgress = 0;
}
void menuPrev() {
  if (isSliding) return;
  slideDir = -1;
  isSliding = true;
  slideProgress = 0;
}
void configNextItem() {
  configIndex++;
  if (configIndex >= CONFIG_COUNT) configIndex = 0;
}
// ========== GUARDAR CONFIGURACIÓN ==========
void saveConfigToEEPROM() {
  // Usar direcciones fijas y separadas
  EEPROM.write(200, soundEnabled ? 1 : 0);
  EEPROM.write(201, (uint8_t)gameDifficulty);
  EEPROM.write(202, screensaverEnabled ? 1 : 0);
  
  // Magic number para verificar que los datos son válidos
  EEPROM.write(210, 0xBB);
  
  // IMPORTANTE: commit para ESP32
  if (EEPROM.commit()) {
    // Opcional: pequeño pitido de confirmación
    playBeep(1000, 50);
  }
  
  delay(10);
}

// ========== CARGAR CONFIGURACIÓN ==========
void loadConfigFromEEPROM() {
  // Verificar magic number
  uint8_t magic = EEPROM.read(210);
  
  if (magic == 0xBB) {
    // Datos válidos, cargar
    uint8_t sound = EEPROM.read(200);
    uint8_t diff = EEPROM.read(201);
    uint8_t screensaver = EEPROM.read(202);
    
    // Validar y asignar
    if (sound == 0 || sound == 1) {
      soundEnabled = (sound == 1);
    }
    
    if (diff >= DIFF_EASY && diff <= DIFF_HARD) {
      gameDifficulty = (Difficulty)diff;
    }
    
    if (screensaver == 0 || screensaver == 1) {
      screensaverEnabled = (screensaver == 1);
    }
  } else {
    // Primera ejecución: valores por defecto
    soundEnabled = true;
    gameDifficulty = DIFF_NORMAL;
    screensaverEnabled = true;
    
    // Guardar valores por defecto
    saveConfigToEEPROM();
  }
}
void configChangeValue() {
  switch (configItems[configIndex].type) {

    case CONFIG_SOUND:
      soundEnabled = !soundEnabled;
      saveConfigToEEPROM();
      break;

    case CONFIG_DIFFICULTY:
      gameDifficulty = (Difficulty)((gameDifficulty + 1) % 3);
      saveConfigToEEPROM();
      break;
    case CONFIG_SCREENSAVER:
      screensaverEnabled = !screensaverEnabled;
      saveConfigToEEPROM();  
      break;
  }
}
 // ================= BOTONES =================
const uint32_t BTN_DEBOUNCE = 50;
bool lastLeft   = HIGH;
bool lastRight  = HIGH;
uint32_t lastLeftTime   = 0;
uint32_t lastRightTime  = 0;
bool leftPressed() {
  bool now = digitalRead(BTN_LEFT);
  if (lastLeft == HIGH && now == LOW && millis() - lastLeftTime > BTN_DEBOUNCE) {
    lastLeftTime = millis();
    lastLeft = now;
    return true;
  }
  lastLeft = now;
  return false;
}
bool rightPressed() {
  bool now = digitalRead(BTN_RIGHT);
  if (lastRight == HIGH && now == LOW && millis() - lastRightTime > BTN_DEBOUNCE) {
    lastRightTime = millis();
    lastRight = now;
    return true;
  }
  lastRight = now;
  return false;
}
// ================== DIBUJO MENU ===================
void drawBall(int x, int y, int r, bool center = false) {
  display.drawDisc(x, y, r);
  if (center) display.drawCircle(x, y, r + 2);
}
void drawMenuCarousel() {
  display.clearBuffer();

  const int centerX = 64;
  const int baseY   = 28;
  const int spacing = 34;

  const int rCenter = 12;
  const int rSide   = 8;

  // animación slide
  unsigned long now = millis();
  if (isSliding && now - lastAnim > 16) {
    slideProgress += 0.22f;
    lastAnim = now;

    if (slideProgress >= 1.0f) {
      currentIndex = idx(currentIndex + slideDir);
      isSliding = false;
      slideProgress = 0;
    }
  }
  int slideOffset = isSliding ? slideProgress * spacing * slideDir : 0;
  display.setDrawColor(1);
  // laterales
  drawBall(centerX - spacing + slideOffset, baseY, rSide);
  drawBall(centerX + spacing + slideOffset, baseY, rSide);
  // centro flotante
  drawBall(centerX + slideOffset,
           baseY + centerFloat(),
           rCenter,
           true);

  // texto
  display.setFont(u8g2_font_6x10_tf);
  int w = display.getStrWidth(menuItems[currentIndex]);
  display.drawStr((128 - w) / 2, 60, menuItems[currentIndex]);
  display.sendBuffer();
}
// ========== DETECTAR BOTONES SIMULTÁNEOS ==========
bool checkBothButtonsPressed() {
  bool leftState = digitalRead(BTN_LEFT) == LOW;
  bool rightState = digitalRead(BTN_RIGHT) == LOW;
  
  if (leftState && rightState) {
    if (!bothButtonsPressedForAlarm) {
      bothButtonsPressTime = millis();
      bothButtonsPressedForAlarm = true;
    }
    return (millis() - bothButtonsPressTime) > bothButtonsHoldTime;
  } else {
    bothButtonsPressedForAlarm = false;
    return false;
  }
}
// ================== BOTON CENTRAL ==================
bool lastCenterBtn = HIGH;
bool centerPressed() {
  bool state = digitalRead(BTN_CENTER);
  if (lastCenterBtn == HIGH && state == LOW) {
    unsigned long now = millis();

    if (now - lastCenterPress > debounceTime) {
      lastCenterPress = now;
      lastCenterBtn = state;
      return true;
    }
  }
  lastCenterBtn = state;
  return false;
}
bool centerEvent;
bool anyButtonPressed() {
  return digitalRead(BTN_LEFT) == LOW || digitalRead(BTN_RIGHT) == LOW || centerEvent;
}
// ========== SONIDO =============
int currentBeepFreq = 0;
unsigned long beepEndTime = 0;
void playBeep(int freq, int dur) {
  if (!soundEnabled) return;

  currentBeepFreq = freq;
  beepEndTime = millis() + dur;

  ledcWriteTone(BUZZER_PIN, freq);
}
void updateBeep() {
  if (currentBeepFreq != 0 && millis() > beepEndTime) {
    ledcWriteTone(BUZZER_PIN, 0);
    currentBeepFreq = 0;
  }
}
// ========== INICIALIZAR ALARMA SLOT ==========
void initAlarmSlot(uint8_t slotIndex) {
  currentAlarmSlot.index = slotIndex;
  currentAlarmSlot.hourTens = alarms[slotIndex].hour / 10;
  currentAlarmSlot.hourUnits = alarms[slotIndex].hour % 10;
  currentAlarmSlot.minuteTens = alarms[slotIndex].minute / 10;
  currentAlarmSlot.minuteUnits = alarms[slotIndex].minute % 10;
  currentAlarmSlot.state = ALARM_SETUP_HOUR_TENS;
  currentAlarmSlot.animOffset = 0;
  currentAlarmSlot.blinkState = true;
  currentAlarmSlot.lastBlink = millis();
  currentAlarmSlot.lastLeftPress = 0;  
  currentAlarmSlot.lastRightPress = 0; 
}

// ========== DIBUJAR MENÚ DE SLOTS DE ALARMA ==========
void drawAlarmMenu() {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  
  // Título
  const char* title = "SELECCIONAR ALARMA";
  int titleWidth = display.getStrWidth(title);
  display.setCursor((128 - titleWidth) / 2, 10);
  display.print(title);
  
  // Dibujar 4 slots
  for (int i = 0; i < MAX_ALARMS; i++) {
    int y = 22 + i * 10;
    
    // Resaltar slot seleccionado
    if (i == selectedAlarmSlot) {
      display.drawBox(0, y - 7, 128, 10);
      display.setDrawColor(0);
    }
    
    // Mostrar slot
    display.setCursor(15, y);
    display.print("Alarma ");
    display.print(i + 1);
    display.print(": ");
    
    if (alarms[i].enabled) {
      char timeStr[6];
      sprintf(timeStr, "%02d:%02d", alarms[i].hour, alarms[i].minute);
      display.print(timeStr);
    } else {
      display.print("--:--");
    }
    
    display.setDrawColor(1);
  }
  
  // Instrucciones
  display.setCursor(2, 62);
  display.print("L:Sel R:Config C:Menu");
  
  display.sendBuffer();
}

// ========== ANIMACIÓN DE DIGITOS ==========
void drawDigitWithAnimation(int x, int y, int digit, int offset, bool selected) {
  char buf[2] = {(char)('0' + digit), 0};
  
  // Si está seleccionado y parpadeando
  if (selected && currentAlarmSlot.blinkState) {
    display.setDrawColor(0);
    display.drawBox(x - 2, y - 10, 12, 12);
    display.setDrawColor(1);
  }
  
  // Dibujar dígito principal
    // Si está seleccionado, dibujar fondo
    if (selected && currentAlarmSlot.blinkState) {
      display.setDrawColor(0);
      display.drawBox(x - 3, y - 13, 14, 16);
      display.setDrawColor(1);
      display.drawFrame(x - 3, y - 13, 14, 16); // Borde para mejor visibilidad
    }
    
    // Dibujar dígito principal
    display.setCursor(x, y + offset);
    display.print(buf);
  
  // Dibujar dígito superior si hay offset negativo
  if (offset < 0) {
    int nextDigit = (digit + 1) % 10;
    buf[0] = '0' + nextDigit;
    display.setCursor(x, y + offset + 10);
    display.print(buf);
  }
  
  // Dibujar dígito inferior si hay offset positivo
  if (offset > 0) {
    int prevDigit = (digit - 1 + 10) % 10;
    buf[0] = '0' + prevDigit;
    display.setCursor(x, y + offset - 10);
    display.print(buf);
  }
}

// ========== DIBUJAR CONFIGURACIÓN DE ALARMA ==========
void drawAlarmSetup() {
  unsigned long now = millis();
  
  // Actualizar parpadeo
  if (now - currentAlarmSlot.lastBlink > 500) {
    currentAlarmSlot.blinkState = !currentAlarmSlot.blinkState;
    currentAlarmSlot.lastBlink = now;
  }
  
  // Actualizar animación
  if (currentAlarmSlot.animOffset != 0) {
    if (now - currentAlarmSlot.lastAnim > 20) {
      if (currentAlarmSlot.animOffset > 0) {
        currentAlarmSlot.animOffset -= 2;
        if (currentAlarmSlot.animOffset <= 0) currentAlarmSlot.animOffset = 0;
      } else {
        currentAlarmSlot.animOffset += 2;
        if (currentAlarmSlot.animOffset >= 0) currentAlarmSlot.animOffset = 0;
      }
      currentAlarmSlot.lastAnim = now;
    }
  }
  
  display.clearBuffer();
  display.setFont(u8g2_font_10x20_tf);
  
  // Título
  display.setFont(u8g2_font_6x10_tf);
  display.setCursor(15, 12);
  display.print("Alarma ");
  display.print(currentAlarmSlot.index + 1);
  
  // Marco para los dígitos
  display.drawFrame(20, 20, 88, 30);
  
  // Dígitos de hora
  display.setFont(u8g2_font_10x20_tf);
  
  // Hora (decenas)
  drawDigitWithAnimation(30, 42, 
    currentAlarmSlot.hourTens, 
    (currentAlarmSlot.state == ALARM_SETUP_HOUR_TENS) ? currentAlarmSlot.animOffset : 0,
    currentAlarmSlot.state == ALARM_SETUP_HOUR_TENS);
  
  // Hora (unidades)
  drawDigitWithAnimation(48, 42,
    currentAlarmSlot.hourUnits,
    (currentAlarmSlot.state == ALARM_SETUP_HOUR_UNITS) ? currentAlarmSlot.animOffset : 0,
    currentAlarmSlot.state == ALARM_SETUP_HOUR_UNITS);
  
  // Separador :
  display.setCursor(66, 42);
  display.print(":");
  
  // Minutos (decenas)
  drawDigitWithAnimation(78, 42,
    currentAlarmSlot.minuteTens,
    (currentAlarmSlot.state == ALARM_SETUP_MINUTE_TENS) ? currentAlarmSlot.animOffset : 0,
    currentAlarmSlot.state == ALARM_SETUP_MINUTE_TENS);
  
  // Minutos (unidades)
  drawDigitWithAnimation(96, 42,
    currentAlarmSlot.minuteUnits,
    (currentAlarmSlot.state == ALARM_SETUP_MINUTE_UNITS) ? currentAlarmSlot.animOffset : 0,
    currentAlarmSlot.state == ALARM_SETUP_MINUTE_UNITS);
  
  // Indicador de slot activo
  if (alarms[currentAlarmSlot.index].enabled) {
    display.setFont(u8g2_font_6x10_tf);
    display.setCursor(65, 12);
    display.print("Activada");
  }
  
  // Instrucciones
  display.setFont(u8g2_font_5x7_tf);
  display.setCursor(4, 60);
  display.print("L:Cambiar R:Sig L+R:Guardar");
  display.setFont(u8g2_font_6x10_tf); 
  display.sendBuffer();
}

// ========== DIBUJAR ALARMA ACTIVA ==========
void drawAlarmActive() {
  unsigned long now = millis();
  
  // Parpadeo del texto
  bool blink = (now / 500) % 2;
  
  display.clearBuffer();
  
  // Texto grande "ALARMA!"
  display.setFont(u8g2_font_logisoso32_tf);
  int w = display.getUTF8Width("ALARMA!");
  display.setCursor((128 - w) / 2, 35);
  if (blink) {
    display.setDrawColor(1);
    display.print("ALARMA!");
  } else {
    // Efecto de borde cuando no se muestra el texto
    display.drawFrame(20, 10, 88, 30);
  }
  // Instrucción para desactivar
  display.setFont(u8g2_font_6x10_tf);
  display.setCursor(4, 60);
  display.print("L y R para detener");
  // Hora actual
  display.setFont(u8g2_font_6x10_tf);
  display.setCursor(4, 50);
  display.print("Hora: ");
  DateTime now_time = rtc.now();
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", now_time.hour(), now_time.minute());
  display.print(timeStr);
  display.sendBuffer();
}
// ========== VERIFICAR ALARMAS ==========
void checkAlarms() {
  if (millis() - lastAlarmCheck < ALARM_CHECK_INTERVAL) return;
  lastAlarmCheck = millis();
  
  // Si ya hay una alarma activa, no verificar otras
  if (currentScreen == SCREEN_ALARM_ACTIVE) return;
  
  DateTime now = rtc.now();
  uint8_t currentHour = now.hour();
  uint8_t currentMinute = now.minute();
  
  for (int i = 0; i < MAX_ALARMS; i++) {
    if (alarms[i].enabled) {
      // Verificar snooze
      if (alarms[i].snoozed) {
        if (millis() > alarms[i].snoozeUntil) {
          alarms[i].snoozed = false;
        } else {
          continue; // Alarma en snooze, saltar
        }
      }
      
      // Verificar si es la hora exacta
      if (alarms[i].hour == currentHour && alarms[i].minute == currentMinute) {
        // Activar alarma
        triggerAlarm(i);
        break;
      }
    }
  }
}
// ========== ACTIVAR ALARMA ==========
void triggerAlarm(uint8_t slotIndex) {
  // Guardar pantalla actual para volver después
  // Podríamos guardar el estado anterior si quisiéramos
  
  currentScreen = SCREEN_ALARM_ACTIVE;
  alarmTriggeredTime = millis();
  
  // Iniciar sonido continuo
  playBeep(ALARM_BUZZER_FREQ, ALARM_BUZZER_DURATION);
  
  // La alarma se dispara inmediatamente
  // El sonido se manejará en el loop con updateBeep() y reactivación
}

// ========== ACTUALIZAR SONIDO DE ALARMA ==========
void updateAlarmSound() {
  if (currentScreen == SCREEN_ALARM_ACTIVE) {
    // Reactivar el beep cada vez que termina
    if (currentBeepFreq == 0) {
      playBeep(ALARM_BUZZER_FREQ, ALARM_BUZZER_DURATION);
    }
  }
}
// ========== PROCESAR CONFIGURACIÓN DE ALARMA ==========
void processAlarmSetup() {
  bool leftNow = digitalRead(BTN_LEFT) == LOW;
  bool rightNow = digitalRead(BTN_RIGHT) == LOW;
  bool bothNow = leftNow && rightNow;
  
  // PRIORIDAD 1: Verificar si están ambos presionados (guardar)
  if (bothNow) {
    // Pequeño debounce visual
    delay(50);
    
    // Verificar que SIGUEN presionados después del delay
    if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_RIGHT) == LOW) {
      
      // Guardar alarma
      uint8_t hour = currentAlarmSlot.hourTens * 10 + currentAlarmSlot.hourUnits;
      uint8_t minute = currentAlarmSlot.minuteTens * 10 + currentAlarmSlot.minuteUnits;
      
      // Validar hora
      if (hour < 24 && minute < 60) {
        alarms[currentAlarmSlot.index].hour = hour;
        alarms[currentAlarmSlot.index].minute = minute;
        alarms[currentAlarmSlot.index].enabled = true;
        alarms[currentAlarmSlot.index].snoozed = false;
        saveAlarmsToEEPROM();
        
        // Feedback
        playBeep(1500, 200);
        
        // Esperar a que suelten los botones antes de salir
        while (digitalRead(BTN_LEFT) == LOW || digitalRead(BTN_RIGHT) == LOW) {
          delay(10);
        }
        
        // Volver al menú
        currentScreen = SCREEN_ALARM_MENU;
        lastCenterBtn = HIGH;
      }
    }
    return; // Salir para no procesar acciones individuales
  }
  
  // PRIORIDAD 2: Procesar botones individuales SOLO si NO están ambos presionados
  
  // Detectar flancos de bajada (presión) con debounce
  static unsigned long lastLeftTime = 0;
  static unsigned long lastRightTime = 0;
  static bool lastLeftState = HIGH;
  static bool lastRightState = HIGH;
  const unsigned long debounceTime = 250; // 250ms entre pulsos
  
  unsigned long now = millis();
  
  // Botón izquierdo (cambiar valor)
  if (leftNow && lastLeftState == HIGH && (now - lastLeftTime > debounceTime)) {
    lastLeftTime = now;
    
    playBeep(800, 20);
    
    // Iniciar animación
    currentAlarmSlot.animOffset = 10;
    
    switch (currentAlarmSlot.state) {
      case ALARM_SETUP_HOUR_TENS:
        currentAlarmSlot.hourTens = (currentAlarmSlot.hourTens + 1) % 3;
        if (currentAlarmSlot.hourTens == 2 && currentAlarmSlot.hourUnits > 3) {
          currentAlarmSlot.hourUnits = 0;
        }
        break;
        
      case ALARM_SETUP_HOUR_UNITS:
        {
          uint8_t maxUnits = (currentAlarmSlot.hourTens == 2) ? 3 : 9;
          currentAlarmSlot.hourUnits = (currentAlarmSlot.hourUnits + 1) % (maxUnits + 1);
        }
        break;
        
      case ALARM_SETUP_MINUTE_TENS:
        currentAlarmSlot.minuteTens = (currentAlarmSlot.minuteTens + 1) % 6;
        break;
        
      case ALARM_SETUP_MINUTE_UNITS:
        currentAlarmSlot.minuteUnits = (currentAlarmSlot.minuteUnits + 1) % 10;
        break;
    }
  }
  
  // Botón derecho (cambiar dígito seleccionado)
  if (rightNow && lastRightState == HIGH && (now - lastRightTime > debounceTime)) {
    lastRightTime = now;
    
    playBeep(800, 30);
    
    switch (currentAlarmSlot.state) {
      case ALARM_SETUP_HOUR_TENS:
        currentAlarmSlot.state = ALARM_SETUP_HOUR_UNITS;
        break;
      case ALARM_SETUP_HOUR_UNITS:
        currentAlarmSlot.state = ALARM_SETUP_MINUTE_TENS;
        break;
      case ALARM_SETUP_MINUTE_TENS:
        currentAlarmSlot.state = ALARM_SETUP_MINUTE_UNITS;
        break;
      case ALARM_SETUP_MINUTE_UNITS:
        currentAlarmSlot.state = ALARM_SETUP_HOUR_TENS;
        break;
    }
  }
  
  // Actualizar estados para detección de flanco
  lastLeftState = leftNow;
  lastRightState = rightNow;
}

// ========== PROCESAR MENÚ DE ALARMAS ==========
void processAlarmMenu() {
  if (leftPressed()) {
    playBeep(600, 30);
    // Seleccionar slot actual
    currentScreen = SCREEN_ALARM_SETUP;
    initAlarmSlot(selectedAlarmSlot);
  }
  
  if (rightPressed()) {
    playBeep(600, 20);
    // Cambiar slot seleccionado
    selectedAlarmSlot = (selectedAlarmSlot + 1) % MAX_ALARMS;
  }
  
  if (centerEvent) {
    // Volver al menú principal
    currentScreen = SCREEN_MENU;
    lastCenterBtn = HIGH;
  }
}

// ========== PROCESAR ALARMA ACTIVA ==========
void processAlarmActive() {
  // Verificar si hay que desactivar con ambos botones
  if (checkBothButtonsPressed()) {
    // Detener alarma actual
    for (int i = 0; i < MAX_ALARMS; i++) {
      if (alarms[i].enabled) {
        DateTime now = rtc.now();
        if (alarms[i].hour == now.hour() && alarms[i].minute == now.minute()) {
          // Opción 1: Desactivar completamente
          alarms[i].enabled = false;
          
          // Opción 2: Snooze (comentar la línea de arriba y descomentar estas)
          // alarms[i].snoozed = true;
          // alarms[i].snoozeUntil = millis() + ALARM_SNOOZE_TIME;
          
          saveAlarmsToEEPROM();
          break;
        }
      }
    }
    
    // Detener sonido y volver al menú
    ledcWriteTone(BUZZER_PIN, 0);
    currentBeepFreq = 0;
    currentScreen = SCREEN_MENU;
    lastCenterBtn = HIGH;
  }
  
  // Actualizar sonido (el updateBeep() ya lo hace, pero necesitamos reactivarlo)
  updateAlarmSound();
}
// ========== CONFIGURACION ==========
void drawConfigScreen() {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  for (uint8_t i = 0; i < CONFIG_COUNT; i++) {
    int y = 18 + i * 14;
    // Indicador de selección
    if (i == configIndex) {
      display.drawBox(0, y - 10, 128, 12);
      display.setDrawColor(0);
    } else {
      display.setDrawColor(1);
    }
    display.setCursor(4, y);
    display.print(configItems[i].name);
    display.print(": ");
    // Mostrar valor
    if (configItems[i].type == CONFIG_SOUND) {
      display.print(soundEnabled ? "ON" : "OFF");
    }
    if (configItems[i].type == CONFIG_DIFFICULTY) {
      if (gameDifficulty == DIFF_EASY)   display.print("FACIL");
      if (gameDifficulty == DIFF_NORMAL) display.print("NORMAL");
      if (gameDifficulty == DIFF_HARD)   display.print("DIFICIL");
    } 
    if (configItems[i].type == CONFIG_SCREENSAVER){
      display.print(screensaverEnabled ? "ON" : "OFF");
    }
  }
  display.setDrawColor(1);
  display.sendBuffer();
}
// ========== PREPARACION SALVAPANTALLAS ========
void enterScreenSaver() {
  activeSaver = (ScreenSaverID)random(SAVER_COUNT);
  if (activeSaver == SAVER_STARS){
    initStars();
  }
  else if (activeSaver == SAVER_MATRIX){
    initMatrix();
  }
  display.clearBuffer();
  display.sendBuffer();
}
// ============= SALVAPANTALLAS 1 =============
void initStars() {
  for (int i = 0; i < STAR_COUNT; i++) {
    stars[i].x = random(-64, 64);
    stars[i].y = random(-32, 32);
    stars[i].z = random(1, 64);
  }
}
void drawScreensaver() {
  display.clearBuffer();

  warpAngle += 0.0005;

for (int i = 0; i < STAR_COUNT; i++) {
  Star &s = stars[i];

  float z1 = s.z;
  float z2 = s.z + 0.9;  // posición anterior

  s.z -= 0.9;

  if (s.z <= 0.5) {
    s.x = random(-64, 64);
    s.y = random(-32, 32);
    s.z = random(32, 64);   // 👈 IMPORTANTE: no siempre 64
    continue;
  }

  float angle1 = warpAngle + (64 - z1) * 0.015;
  float angle2 = warpAngle + (64 - z2) * 0.015;

  float cx1 = s.x * cos(angle1) - s.y * sin(angle1);
  float cy1 = s.x * sin(angle1) + s.y * cos(angle1);

  float cx2 = s.x * cos(angle2) - s.y * sin(angle2);
  float cy2 = s.x * sin(angle2) + s.y * cos(angle2);

  int px1 = cx1 / z1 * 64 + 64;
  int py1 = cy1 / z1 * 32 + 32;

  int px2 = cx2 / z2 * 64 + 64;
  int py2 = cy2 / z2 * 32 + 32;

  int thickness = map(z1, 1, 64, 3, 1);

  if (px1 >= 0 && px1 < 128 && py1 >= 0 && py1 < 64) {
    for (int t = 0; t < thickness; t++) {
      display.drawLine(px1 + t, py1, px2 + t, py2);
    }
  }
}

  display.sendBuffer();
}
// =========== SALVAPANTALLAS 2 ==============
void initMatrix() {
  for (int i = 0; i < MATRIX_COLS; i++) {
    matrix[i].y = random(-64, 0);
    matrix[i].speed = random(2, 4);
    matrix[i].length = random(4, 10);
  }
}
char matrixChar() {
  const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%&";
  return chars[random(sizeof(chars) - 1)];
}
void drawMatrixSaver() {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tf);

  for (int i = 0; i < MATRIX_COLS; i++) {
    int x = i * FONT_W;

    // cabeza
    display.setCursor(x, matrix[i].y);
    display.print(matrixChar());

    // cola (estela)
    for (int j = 1; j < matrix[i].length; j++) {
      int y = matrix[i].y - j * FONT_H;
      if (y < 0) break;

      display.setCursor(x, y);
      display.print(matrixChar());
    }

    // movimiento
    matrix[i].y += matrix[i].speed;

    // reset columna
    if (matrix[i].y - matrix[i].length * FONT_H > SCREEN_H) {
      matrix[i].y = random(-SCREEN_H, 0);
      matrix[i].speed = random(1, 3);
      matrix[i].length = random(4, 8);
    }
  }

  display.sendBuffer();
}
void drawScreensaverTransition() {
  if (millis() - lastTransitionFrame < TRANSITION_SPEED) return;
  lastTransitionFrame = millis();

  display.clearBuffer();

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    int srcY = y - transitionOffset;
    if (srcY < 0 || srcY >= SCREEN_HEIGHT) continue;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
      if (getPixelFromTransition(x, srcY)) {
        display.drawPixel(x, y);
      }
    }
  }

  display.sendBuffer();

  transitionOffset++;

  if (transitionOffset >= SCREEN_HEIGHT) {
    screensaverTransition = false;
    transitionOffset = 0;
    currentScreen = SCREEN_SCREENSAVER;
    enterScreenSaver();
  }
}
bool getPixelFromTransition(int x, int y) {
  int index = x + (y / 8) * SCREEN_WIDTH;
  return transitionBuffer[index] & (1 << (y & 7));
}
void startScreensaverTransition() {
  memcpy(transitionBuffer,
         display.getBufferPtr(),
         sizeof(transitionBuffer));

  screensaverTransition = true;
  transitionOffset = 0;
  lastTransitionFrame = millis();
}
// ================== RELOJ ==================
void drawClock() {
  static uint32_t lastRead = 0;
  static DateTime now;

  if (millis() - lastRead > 200) {
    now = rtc.now();
    lastRead = millis();
  }
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d",
          now.hour(), now.minute(), now.second());
  display.clearBuffer();
  display.setFont(u8g2_font_logisoso24_tf);  // ⬅️ MÁS CHICO
  int w = display.getUTF8Width(timeStr);
  int x = (128 - w) / 2;
  display.setCursor(x, 42);
  display.print(timeStr);
  display.sendBuffer();
}
void drawAlarm() {
  display.clearBuffer();
  display.drawFrame(0, 0, 128, 64);
  display.drawLine(0, 0, 127, 63);
  display.drawLine(0, 63, 127, 0);
  display.drawTriangle(0, 0, 64, 64, 128, 0);
  playBeep(1200, 500);
  display.sendBuffer();
}
// =============== PECERA ===============
void drawPlants() {
  display.setDrawColor(1);
  // Planta 1 - curva
  for (int i = 0; i < 5; i++) {
    display.drawLine(
      10 + i, 63,
      10 + i - (i % 2 == 0 ? 2 : -2),
      50 - i * 2
    );
  }
  // Planta 2 - más alta
  for (int i = 0; i < 6; i++) {
    display.drawLine(
      30 + i, 63,
      30 + i + (i % 2 == 0 ? 1 : -1),
      47 - i * 2
    );
  }
  // Planta 3 - más ancha
  for (int i = 0; i < 7; i++) {
    display.drawLine(
      60 + i, 63,
      60 + i + (i % 2 == 0 ? 2 : -1),
      45 - i * 2
    );
  }
  // Planta 4 - más recta
  for (int i = 0; i < 4; i++) {
    display.drawLine(
      100 + i, 63,
      100 + i,
      48 - i * 3
    );
  }
}
void drawDoubleBorder() {
  display.setDrawColor(1);
  display.drawFrame(0, 0, 128, 64);
  display.drawFrame(2, 2, 124, 60);
}
void drawFood(Food &f) {
  if (f.active) {
    display.drawDisc(f.x, f.y, 3);
  }
}
void drawBubble(Bubble &b) {
  display.drawCircle(b.x, b.y, 2);
  display.drawPixel(b.x - 1, b.y - 1); // brillo
}
void initAquarium() {
  for (int i = 0; i < MAX_FISH; i++) {
    fishes[i].x = random(20, SCREEN_WIDTH - 20);
    fishes[i].y = random(15, SCREEN_HEIGHT - 15);
    fishes[i].size = random(2, 4);
    fishes[i].speed = random(1, 2);
    fishes[i].direction = random(0, 2);
    fishes[i].eaten = 0;
    fishes[i].targetFood = -1;
  }
  for (int i = 0; i < MAX_FOOD; i++)
    foods[i].active = false;
  for (int i = 0; i < MAX_BUBBLES; i++) {
  bubbles[i].x = random(6, SCREEN_WIDTH - 6);
  bubbles[i].y = random(10, SCREEN_HEIGHT - 6);
  bubbles[i].speed = random(1, 3);  // NUNCA 0
}
}
void drawFish(Fish &f) {
  int s = f.size;
  // cuerpo
  display.drawDisc(f.x, f.y, s);
  // cola
  if (f.direction) {
    display.drawTriangle(
      f.x - s, f.y,
      f.x - s - s, f.y - s,
      f.x - s - s, f.y + s
    );
  } else {
    display.drawTriangle(
      f.x + s, f.y,
      f.x + s + s, f.y - s,
      f.x + s + s, f.y + s
    );
  }
  // ojo
  display.drawPixel(
    f.x + (f.direction ? 1 : -1),
    f.y - 1
  );
}
void spawnFood() {
  for (int i = 0; i < MAX_FOOD; i++) {
    if (!foods[i].active) {
      foods[i].x = random(10, SCREEN_WIDTH - 10);
      foods[i].y = 5;
      foods[i].active = true;
      // asignar pez aleatorio libre
      int candidates[MAX_FISH];
      int count = 0;
      for (int f = 0; f < MAX_FISH; f++)
        if (fishes[f].targetFood < 0)
          candidates[count++] = f;
      if (count > 0) {
        int chosen = candidates[random(count)];
        fishes[chosen].targetFood = i;
      }
      break; // solo una comida por pulsación
    }
  }
}
void updateAquarium() {
  display.clearBuffer();
  drawDoubleBorder();
  drawPlants();
// burbujas
for (int i = 0; i < MAX_BUBBLES; i++) {
  Bubble &b = bubbles[i];

  drawBubble(b);

  b.y -= b.speed;

  if (b.y <= 5) {
    b.y = SCREEN_HEIGHT - 5;
    b.x = random(6, SCREEN_WIDTH - 6);
    b.speed = random(1, 3);
  }
}
  // comida
  for (int i = 0; i < MAX_FOOD; i++) {
    if (!foods[i].active) continue;
    foods[i].y++;
    if (foods[i].y > SCREEN_HEIGHT - 2)
      foods[i].active = false;
    drawFood(foods[i]);
  }
  // peces
  for (int i = 0; i < MAX_FISH; i++) {
    Fish &f = fishes[i];
    // perder objetivo si ya no existe
    if (f.targetFood >= 0 && !foods[f.targetFood].active) {
      f.targetFood = -1;
    }
    // perseguir comida
    if (f.targetFood >= 0 && foods[f.targetFood].active) {
      int j = f.targetFood;
      int dx = foods[j].x - f.x;
      int dy = foods[j].y - f.y;
      if (abs(dx) < 3 && abs(dy) < 3) {
        foods[j].active = false;
        f.eaten++;
        playBeep(1200, 40);
        if (f.eaten >= 3) {
          f.size++;
          f.eaten = 0;
        }
        f.targetFood = -1;
      } else {
        f.direction = (dx >= 0);

        if (foods[j].x > f.x + 1) {
        f.x += f.speed;
        f.direction = true;
      }
      else if (foods[j].x < f.x - 1) {
       f.x -= f.speed;
      f.direction = false;
       }
        if (dy != 0)
          f.y += (dy > 0 ? 1 : -1) * max(1, f.speed / 2);
      }
    }
    // nado libre
    else {
      if (random(100) < 5)  f.direction = !f.direction;
      if (random(100) < 10) f.y += random(-1, 2);
      f.x += f.direction ? f.speed : -f.speed;
    }
   if (f.speed < 1) f.speed = 1; // En caso de corrupcion de velocidad
    // límites suaves
// límites con rebote REAL
if (f.x <= 5) {
  f.x = 5;
  f.direction = true;
}
if (f.x >= SCREEN_WIDTH - 5) {
  f.x = SCREEN_WIDTH - 5;
  f.direction = false;
}
if (f.y < 10) f.y = 10;
if (f.y > SCREEN_HEIGHT - 10) f.y = SCREEN_HEIGHT - 10;
    if (f.size > 10) f.size = 10;
    drawFish(f);
  }
  if (millis() - lastBubbleSound > 1200) {
  if (random(200) < 30) {
    playBeep(random(500, 800), 30);
    lastBubbleSound = millis();
  }
}
}
// ================== PONG ===================
void resetPong() {
  playerScore = 0;
  aiScore = 0;
  pongState = PONG_PLAY;
  playerY = 32 - PADDLE_H / 2;
  aiY = playerY;
  resetBall();
}
void updateAI() {
  int speed;
  int errorChance;
if (gameDifficulty == DIFF_EASY)   speed = 1;
if (gameDifficulty == DIFF_NORMAL) speed = 2;
if (gameDifficulty == DIFF_HARD)   speed = 3;
  int target = ballY - PADDLE_H / 2;
  if (aiY < target) aiY += speed;
  if (aiY > target) aiY -= speed;
  if (random(100) < errorChance)
    aiY += random(-3, 4);
  aiY = constrain(aiY, 0, 64 - PADDLE_H);
}
void updatePong() {
  if (millis() - lastPongFrame < 16) return; // ~60 FPS
  lastPongFrame = millis();
  // jugador
  if (digitalRead(BTN_LEFT) == LOW)  playerY -= 2;
  if (digitalRead(BTN_RIGHT) == LOW) playerY += 2;
  playerY = constrain(playerY, 0, 64 - PADDLE_H);
  updateAI();
  // mover pelota
  ballX += ballVX;
  ballY += ballVY;
  // rebote arriba/abajo
  if (ballY <= 0 || ballY >= 62){
     ballVY = -ballVY;
     if (soundEnabled) {
  playBeep(1200, 40);
}
}
// pala jugador
if (ballX <= 6 &&
    ballY >= playerY &&
    ballY <= playerY + PADDLE_H) {
  ballVX = abs(ballVX);
  if (soundEnabled) {
  playBeep(1200, 40);
}
  int impact = ballY - (playerY + PADDLE_H / 2);
  ballVY = constrain(impact / 3, -3, 3);
}
// pala IA
if (ballX >= 120 &&
    ballY >= aiY &&
    ballY <= aiY + PADDLE_H) {
  ballVX = -abs(ballVX);
  if (soundEnabled) {
  playBeep(1200, 40);
}
  int impact = ballY - (aiY + PADDLE_H / 2);
  ballVY = constrain(impact / 3, -3, 3);
}
  // punto
if (ballX < 0) {
  aiScore++;
  if (soundEnabled) {
  playBeep(700, 40);
}
  if (aiScore >= WIN_SCORE) {
    pongState = PONG_LOSE;
    if (soundEnabled) {
  playBeep(400, 40);
}
  } else {
    resetBall();
  }
}
if (ballX > 128) {
  playerScore++;
  if (playerScore >= WIN_SCORE) {
    pongState = PONG_WIN;
    if (soundEnabled) {
  playBeep(1500, 40);
}
  } else {
    resetBall();
  }
}
}
void drawPong() {
  display.clearBuffer();
  // línea central
  for (int y = 0; y < 64; y += 4)
    display.drawVLine(64, y, 2);
  // palas
  display.drawBox(2, playerY, PADDLE_W, PADDLE_H);
  display.drawBox(124, aiY, PADDLE_W, PADDLE_H);
  // pelota
  display.drawBox(ballX, ballY, BALL_SIZE, BALL_SIZE);
  // score
  display.setFont(u8g2_font_6x10_tf);
  display.setCursor(52, 10);
  display.print(playerScore);
  display.setCursor(72, 10);
  display.print(aiScore);
  display.sendBuffer();
}
void resetBall() {
  ballX = 64;
  ballY = random(10, 54);
  ballVX = random(0, 2) ? 2 : -2;
  // velocidad Y nunca cero
  do {
    ballVY = random(-2, 3);
  } while (ballVY == 0);
}
void drawPongEnd(bool win) {
  static int anim = 0;
  anim = (anim + 1) % 64;
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.setCursor(36, 20);
  display.print(win ? "GANASTE!" : "PERDISTE");
  display.setCursor(20, 40);
  display.print("Centro para salir");
  // animación simple
  display.drawDisc(64, 10 + sin(anim * 0.1f) * 5, 4);
  display.sendBuffer();
}

// ================== SETUP ====================
void setup() {
  pinMode(BTN_LEFT,   INPUT_PULLUP);
  pinMode(BTN_CENTER, INPUT_PULLUP);
  pinMode(BTN_RIGHT,  INPUT_PULLUP);
  ledcAttach(BUZZER_PIN, 2000, 8);   // 2kHz, 8 bits
  randomSeed(analogRead(0) ^ millis());
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // <------ SOLO UNA VEZ
  display.begin();
  display.setPowerSave(0);
  display.setContrast(60);
  initAquarium();
  EEPROM.begin(1024);
  // Verificar si es primera vez
  if (EEPROM.read(100) != 0xAA) {
    // Inicializar todo con ceros
    for (int i = 0; i < MAX_ALARMS; i++) {
      alarms[i].hour = 0;
      alarms[i].minute = 0;
      alarms[i].enabled = false;
      alarms[i].snoozed = false;
      alarms[i].snoozeUntil = 0;
    }
    saveAlarmsToEEPROM(); // Guardar estado inicial
  } else {
    loadAlarmsFromEEPROM();
    loadConfigFromEEPROM();
  }
  for (int i = 0; i < MAX_ALARMS; i++) {
    if (alarms[i].hour > 23 || alarms[i].minute > 59) {
      alarms[i].hour = 0;
      alarms[i].minute = 0;
      alarms[i].enabled = false;
    }
  }
}
// ================== LOOP =====================
void loop() {
  if(currentScreen != SCREEN_ALARM_ACTIVE) {centerEvent = centerPressed();}
  updateBeep();
  if(anyButtonPressed()){
    lastInteractionMillis = millis();
    if(currentScreen == SCREEN_SCREENSAVER){
      currentScreen = SCREEN_MENU;
    }
  }
if (screensaverEnabled &&
    currentScreen != SCREEN_SCREENSAVER &&
    !screensaverTransition &&
    millis() - lastInteractionMillis > SCREENSAVER_TIMEOUT && currentScreen != SCREEN_ALARM_ACTIVE) {
  screensaverTransition = true;
  transitionOffset = 0;
  startScreensaverTransition();  
  }// IF para entrar al screensaver
  if (screensaverTransition) {
  drawScreensaverTransition();
  return;
 }
  if (currentScreen != SCREEN_ALARM_ACTIVE) {
    checkAlarms();
  }
  switch (currentScreen){
    case SCREEN_MENU:
  if (!digitalRead(BTN_LEFT))  menuPrev();
  if (!digitalRead(BTN_RIGHT)) menuNext();    
    drawMenuCarousel();
  if (centerEvent) {
     if (strcmp(menuItems[currentIndex], "Reloj") == 0) {
    currentScreen = SCREEN_CLOCK;
    lastCenterBtn = HIGH;   // ⬅️ RESET FLANCO
  }
     else if (strcmp(menuItems[currentIndex], "Alarma") == 0) {
    currentScreen = SCREEN_ALARM_MENU;
    lastCenterBtn = HIGH;   // ⬅️ RESET FLANCO
    selectedAlarmSlot = 0;
  }
     else if (strcmp(menuItems[currentIndex], "Pong") == 0) {
    currentScreen = SCREEN_PONG;
    lastCenterBtn = HIGH;
    resetPong();
     }
     else if (strcmp(menuItems[currentIndex], "Configuracion") == 0) {
    currentScreen = SCREEN_CONFIG;
    lastCenterBtn = HIGH;
}
     else if (strcmp(menuItems[currentIndex], "Pecera") == 0){
      currentScreen = SCREEN_PECERA;
      lastCenterBtn = HIGH;
     }
     else if (strcmp(menuItems[currentIndex], "Salvapantallas") == 0){
      startScreensaverTransition();  // IF para entrar al screensaver
      lastCenterBtn = HIGH;
     }
     else if (strcmp(menuItems[currentIndex], "Pibble") == 0){
      drawwash_my_belly();
      currentScreen = SCREEN_PIBBLE;
      lastCenterBtn = HIGH;
     }
}
  break;
  case SCREEN_CLOCK:
  drawClock();
    if (centerEvent) {
      currentScreen = SCREEN_MENU;
      lastCenterBtn = HIGH;   // ⬅️ RESET FLANCO
    }
  break;
  case SCREEN_ALARM_MENU:
  drawAlarmMenu();
  processAlarmMenu();
    if (centerEvent) {
      currentScreen = SCREEN_MENU;
      lastCenterBtn = HIGH;
    }
  break;
  case SCREEN_ALARM_SETUP:
  drawAlarmSetup();
  processAlarmSetup();
    if (centerEvent){
      currentScreen = SCREEN_MENU;
      lastCenterBtn = HIGH;
    }
  break;
  case SCREEN_ALARM_ACTIVE:
  drawAlarmActive();
  processAlarmActive();
    if (centerEvent){
      currentScreen = SCREEN_MENU;
      lastCenterBtn = HIGH;
    }
  break;
  case SCREEN_PONG:
  if (pongState == PONG_PLAY) {
    updatePong();
    drawPong();
  }
  else if (pongState == PONG_WIN) {
    drawPongEnd(true);
  }
  else if (pongState == PONG_LOSE) {
    drawPongEnd(false);
  }
  if (centerEvent) {
    currentScreen = SCREEN_MENU;
    lastCenterBtn = HIGH;
  }
  break;
  case SCREEN_CONFIG:
  drawConfigScreen();
  if (leftPressed()) {
    configNextItem();
  }
  if (rightPressed()) {
    configChangeValue();
  }
  if (centerEvent) {
    currentScreen = SCREEN_MENU;
    lastCenterBtn = HIGH;
  }
  break;
  case SCREEN_PECERA:
  if (rightPressed())
    spawnFood();
  if (centerEvent){
    currentScreen = SCREEN_MENU;
    lastCenterBtn = HIGH;
  }
  updateAquarium();
  display.sendBuffer();
  break;
  case SCREEN_SCREENSAVER:
  switch(activeSaver){
  case SAVER_MATRIX:
  drawMatrixSaver();
  break;
  case SAVER_STARS:
  drawScreensaver();
  break;
    if (centerEvent){
      currentScreen = SCREEN_MENU;
      lastCenterBtn = HIGH;
    }
  }
  break;
  case SCREEN_PIBBLE:
  if (!pibbleentered){
    drawwash_my_belly();
    estadopibble = 0;
    pibbleentered = true;
  }
   switch(estadopibble){
    case 0:
    if (rightPressed()){
      drawyaaay();
      animTimer = millis();
      estadopibble = 1;
    }
    break;
    case 1:
    if (millis() - animTimer >= 2000){
      drawclean_my_belly();
      estadopibble = 2;
    break;
    case 2:
     if (rightPressed()){
      drawcleean_my_belly();
      estadopibble = 3;
     }
    break;
    case 3: 
    break;
    }
   }
    if (centerEvent){
      pibbleentered = false;
      currentScreen = SCREEN_MENU;
      lastCenterBtn = HIGH;
    }
  break;
  }

}

