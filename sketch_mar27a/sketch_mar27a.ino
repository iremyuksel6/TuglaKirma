#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define POT_PIN A0 // Potansiyometre pin
#define BALL_SIZE 6 // Topun çapı
#define PADDLE_WIDTH 20 // Dikdörtgen çubuğun genişliği
#define PADDLE_HEIGHT 4 // Dikdörtgen çubuğun yüksekliği
#define PADDLE_SPEED 2 // Dikdörtgen çubuğun hareket hızı
#define BALL_SPEED_X 2 // Topun x ekseni hızı
#define BALL_SPEED_Y 2 // Topun y ekseni hızı
#define BRICK_WIDTH 20 // Tuğla genişliği
#define BRICK_HEIGHT 8 // Tuğla yüksekliği
#define NUM_BRICKS 5 // Tuğla sayısı
#define BRICK_GAP 2 // Tuğlalar arasındaki boşluk

int paddlePos = SCREEN_WIDTH / 2 - PADDLE_WIDTH / 2; // Dikdörtgen çubuğun başlangıç pozisyonu
int ballX = SCREEN_WIDTH / 2; // Topun başlangıç pozisyonu (x ekseni)
int ballY = SCREEN_HEIGHT / 2; // Topun başlangıç pozisyonu (y ekseni)
int ballSpeedX = BALL_SPEED_X; // Topun x ekseni hızı
int ballSpeedY = BALL_SPEED_Y; // Topun y ekseni hızı

// Tuğlaların konumlarını ve durumlarını tutacak diziler
int brickX[NUM_BRICKS];
int brickY[NUM_BRICKS];
bool brickAlive[NUM_BRICKS];

bool gameOver = false; // Oyun durumu
bool startGame = false; // Oyunun başladı mı?

void setup() {
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  Serial.begin(9600);

  // OLED ekran başlatma
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 ekran başlatılamadı!"));
    while(true);
  }

  // Potansiyometre için analog girişi ayarlama
  pinMode(POT_PIN, INPUT);

  // Tuğlaların başlangıç konumlarını ayarlama
  int initialBrickX = (SCREEN_WIDTH - (NUM_BRICKS * (BRICK_WIDTH + BRICK_GAP))) / 2;
  for(int i = 0; i < NUM_BRICKS; i++) {
    brickX[i] = initialBrickX + i * (BRICK_WIDTH + BRICK_GAP);
    brickY[i] = 10;
    brickAlive[i] = true;
  }
}

void loop() {
  int up = digitalRead(A1);
  int down = digitalRead(A2);
  int choose = digitalRead(A3);

  // Oyun başlamadıysa seçenekleri göster
  if (!startGame) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.println("BASLAT");
    display.setCursor(10, 30);
    display.println("CIKIS");
    display.display();

    // Butonlarla kontrol
    if (choose == HIGH) {
      startGame = true; // BAŞLAT seçeneği seçildi
    } else if (down == HIGH) {
      // ÇIKIŞ seçeneği seçildi, çıkış yap
      while (true) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(10, 20);
        display.println("OYUNDAN CIKILIYOR...");
        display.display();
        delay(1000); // Bir saniye bekleyip çıkış yapabiliriz.
      }
    }
    // Devam etmeden önce biraz bekle
    delay(100);
    return;
  }

  // Oyun başladıysa devam et
  if (!gameOver) {
    // Potansiyometre değerini okuma
    int potValue = analogRead(POT_PIN);
    // Potansiyometre değerini ekran boyutuna orantılayarak dikdörtgen çubuğun x pozisyonunu güncelleme
    paddlePos = map(potValue, 0, 1023, 0, SCREEN_WIDTH - PADDLE_WIDTH);

    // Ekranı temizleme
    display.clearDisplay();

    // Dikdörtgen çubuğu çizme
    display.fillRect(paddlePos, SCREEN_HEIGHT - PADDLE_HEIGHT, PADDLE_WIDTH, PADDLE_HEIGHT, WHITE);

    // Tuğlaları çizme
    for(int i = 0; i < NUM_BRICKS; i++) {
      if(brickAlive[i]) {
        display.fillRect(brickX[i], brickY[i], BRICK_WIDTH, BRICK_HEIGHT, WHITE);
      }
    }

    // Topu çizme
    display.fillCircle(ballX, ballY, BALL_SIZE, WHITE);

    // Ekranı güncelleme
    display.display();

    // Topun hareketini güncelleme
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Topun çarpışmasını kontrol etme
    if(ballX <= 0 || ballX >= SCREEN_WIDTH) {
      ballSpeedX = -ballSpeedX;
    }
    if(ballY <= 0 || (ballY >= SCREEN_HEIGHT - PADDLE_HEIGHT && ballX > paddlePos && ballX < paddlePos + PADDLE_WIDTH && ballY + BALL_SIZE >= SCREEN_HEIGHT - PADDLE_HEIGHT)) {
      ballSpeedY = -ballSpeedY;
    }
    
    // Tuğlalarla çarpışmayı kontrol etme
    for(int i = 0; i < NUM_BRICKS; i++) {
      if(brickAlive[i] && ballY <= brickY[i] + BRICK_HEIGHT && ballX >= brickX[i] && ballX <= brickX[i] + BRICK_WIDTH) {
        ballSpeedY = -ballSpeedY;
        brickAlive[i] = false; // Tuğlayı yok etme
      }
    }
    
    // Topun alt kenara ulaşmasını kontrol etme
    if (ballY >= SCREEN_HEIGHT) {
      gameOver = true; // Oyunu durdur
    }
  } else { // Oyun bitti
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    display.println("GAME OVER");
    display.display();

    // Oyunu yeniden başlatmak için bekleyin
    delay(2000);
    
    // Oyunu sıfırla
    resetGame();
  }
}

// Oyunu sıfırlama fonksiyonu
void resetGame() {
  // Topun ve tuğlaların başlangıç konumlarını yeniden ayarlama
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  ballSpeedX = BALL_SPEED_X;
  ballSpeedY = BALL_SPEED_Y;

  int initialBrickX = (SCREEN_WIDTH - (NUM_BRICKS * (BRICK_WIDTH + BRICK_GAP))) / 2;
  for(int i = 0; i < NUM_BRICKS; i++) {
    brickX[i] = initialBrickX + i * (BRICK_WIDTH + BRICK_GAP);
    brickAlive[i] = true;
  }

  gameOver = false; // Oyun durumunu sıfırla
}
