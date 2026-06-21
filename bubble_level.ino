#include <M5Cardputer.h>
#include <math.h>

static int CENTER_X = 0;
static int CENTER_Y = 0;
static int LEVEL_R  = 0;
static int BUBBLE_R = 0;
static int TARGET_R = 0;

static constexpr float SMOOTHING       = 0.15f;   // 0..1, higher = snappier
static constexpr float FULL_SCALE_G    = 0.5f;    // ±0.5g maps to full deflection (~30°)
static constexpr float LEVEL_TOL_DEG   = 1.0f;    // "you are level" tolerance
static constexpr uint32_t FRAME_PERIOD_MS = 20;   // ~50 fps

static M5Canvas canvas(&M5.Display);
static float fx = 0.0f, fy = 0.0f;     // low-pass-filtered accel
static float ox = 0.0f, oy = 0.0f;     // calibrated zero offsets
static bool  zeroed = false;

static void handleKeys() {
  if (!M5Cardputer.Keyboard.isChange())  return;
  if (!M5Cardputer.Keyboard.isPressed()) return;

  auto state = M5Cardputer.Keyboard.keysState();
  for (auto c : state.word) {
    if (c == 'c' || c == 'C' || c == ' ') {
      ox = fx;
      oy = fy;
      zeroed = true;
    } else if (c == 'r' || c == 'R') {
      ox = 0.0f;
      oy = 0.0f;
      zeroed = false;
    }
  }
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);            // true = enable keyboard
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);

  if (!M5.Imu.isEnabled()) {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 50);
    M5.Display.print("IMU not detected");
    while (true) { delay(1000); }
  }

  const int w = M5.Display.width();
  const int h = M5.Display.height();
  CENTER_X = w / 2;
  CENTER_Y = h / 2;
  const int shortSide = (w < h) ? w : h;
  LEVEL_R  = shortSide / 2 - 6;
  BUBBLE_R = LEVEL_R / 5;
  TARGET_R = BUBBLE_R / 2 + 2;

  canvas.setColorDepth(16);
  canvas.createSprite(w, h);
}

void loop() {
  M5Cardputer.update();   // updates display, IMU, and keyboard state
  M5.Imu.update();
  handleKeys();

  float ax = 0.0f, ay = 0.0f, az = 1.0f;
  M5.Imu.getAccel(&ax, &ay, &az);

  fx += SMOOTHING * (ax - fx);
  fy += SMOOTHING * (ay - fy);

  const float cx = fx - ox;
  const float cy = fy - oy;

  const float scale = (LEVEL_R - BUBBLE_R) / FULL_SCALE_G;
  int bx = CENTER_X + (int)lroundf(cx * scale);
  int by = CENTER_Y - (int)lroundf(cy * scale);   // IMU Y is inverted vs display Y

  // Clamp bubble inside the ring
  const float dx = (float)(bx - CENTER_X);
  const float dy = (float)(by - CENTER_Y);
  const float dist = sqrtf(dx * dx + dy * dy);
  const float maxDist = (float)(LEVEL_R - BUBBLE_R);
  if (dist > maxDist && dist > 0.0f) {
    bx = CENTER_X + (int)lroundf(dx * maxDist / dist);
    by = CENTER_Y + (int)lroundf(dy * maxDist / dist);
  }

  // Tilt readouts use the calibrated values so they read 0° when zeroed
  const float pitch = atan2f(cy, sqrtf(cx * cx + az * az)) * 180.0f / (float)M_PI;
  const float roll  = atan2f(cx, sqrtf(cy * cy + az * az)) * 180.0f / (float)M_PI;
  const float total = sqrtf(pitch * pitch + roll * roll);
  const bool  level = total < LEVEL_TOL_DEG;

  canvas.fillScreen(TFT_BLACK);

  // Crosshair
  canvas.drawFastHLine(CENTER_X - LEVEL_R, CENTER_Y, 2 * LEVEL_R, TFT_DARKGREY);
  canvas.drawFastVLine(CENTER_X, CENTER_Y - LEVEL_R, 2 * LEVEL_R, TFT_DARKGREY);

  // Outer ring
  canvas.drawCircle(CENTER_X, CENTER_Y, LEVEL_R,     TFT_LIGHTGREY);
  canvas.drawCircle(CENTER_X, CENTER_Y, LEVEL_R - 1, TFT_LIGHTGREY);

  // Center target
  canvas.drawCircle(CENTER_X, CENTER_Y, TARGET_R,
                    level ? TFT_GREEN : TFT_DARKGREY);

  // Bubble
  const uint16_t bubbleColor = level ? TFT_GREEN : TFT_CYAN;
  canvas.fillCircle(bx, by, BUBBLE_R, bubbleColor);
  canvas.drawCircle(bx, by, BUBBLE_R, TFT_WHITE);

  // Numeric tilt readouts
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextSize(1);
  canvas.setCursor(4, 4);
  canvas.printf("Pitch:%+6.1f", pitch);
  canvas.setCursor(4, 14);
  canvas.printf("Roll: %+6.1f", roll);
  canvas.setCursor(4, 24);
  canvas.printf("Tilt: %5.2f", total);

  if (zeroed) {
    canvas.setTextColor(TFT_YELLOW, TFT_BLACK);
    canvas.setCursor(4, 36);
    canvas.print("ZEROED");
  }

  // Bottom hint
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setCursor(4, M5.Display.height() - 10);
  canvas.print("C/space:zero  R:reset");

  canvas.pushSprite(0, 0);
  delay(FRAME_PERIOD_MS);
}
