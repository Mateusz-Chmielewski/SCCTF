#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define RESET_PIN DD6
#define UP_PIN DD5
#define DOWN_PIN DD4
#define SENSOR_PIN A0
#define RELAY_PIN DD3
#define SPEAKER_PIN DD7

#define MAX_CICLE 5
#define OPEN_CYCYLE 5 // example: 1, 2, 3, 4, ...
#define DELAY_MS 200
#define NOISE_DURATION_MS 5000

#define ON_CYCLE_MESSAGE "Wysiew"
#define DEFAULT_MESSAGE "Zamkniete"

int lows[] = {A1, A2};
int highs[] = {A3};

LiquidCrystal_I2C lcd(0x3F, 16, 2);

class InputController
{
public:
  enum Type
  {
    UP, DOWN, RESET, NEXT, NONE
  };

  Type CatchEvent();
  void UpdateCounter();
  void SetRelay();
  void SetSpeaker();
  void PrintStatusOnDisplay();
  void SetHighsAndLows();

private:
  int counter = 0;
  Type last = NONE;
  bool work = true;
  int noiseDuration = NOISE_DURATION_MS;
};

InputController* inputController;

void setup() 
{
  Serial.begin(9600);

  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_PIN, INPUT);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  inputController = new InputController();
  inputController->SetHighsAndLows();
}

void loop() 
{
  delay(DELAY_MS);

  inputController->UpdateCounter();
  inputController->SetRelay();
  inputController->SetSpeaker();
  inputController->PrintStatusOnDisplay();
}

InputController::Type InputController::CatchEvent()
{
  if (!work && !digitalRead(SENSOR_PIN))
  {
    return NEXT;
  }

  if (digitalRead(RESET_PIN))
  {
    return RESET;
  }

  if (digitalRead(UP_PIN))
  {
    return UP;
  }

  if (digitalRead(DOWN_PIN))
  {
    return DOWN;
  }

  return NONE;
}

void InputController::UpdateCounter()
{
  Type event = CatchEvent();
  work = !digitalRead(SENSOR_PIN);

  if (event == last)
  {
    return;
  }

  last = event;

  switch (event)
  {
  case NEXT:
  case UP:
    counter++;
    counter %= MAX_CICLE;
    return;

  case DOWN:
    counter = max(0, counter - 1);
    return;

  case RESET:
    counter = 0;
    return;
  }
}

void InputController::SetRelay()
{
  digitalWrite(RELAY_PIN, counter != OPEN_CYCYLE - 1);
}

void InputController::SetSpeaker()
{
  if (!work || counter != OPEN_CYCYLE - 1)
  {
    noiseDuration = NOISE_DURATION_MS;
    digitalWrite(SPEAKER_PIN, LOW);
    return;
  }

  noiseDuration = max(0, noiseDuration - DELAY_MS);
  digitalWrite(SPEAKER_PIN, noiseDuration > 0);
}

void InputController::PrintStatusOnDisplay()
{
  lcd.clear();
  lcd.setCursor(7, 0);
  lcd.print(counter + 1);
  lcd.setCursor(3, 1);
  lcd.print(digitalRead(RELAY_PIN) ? ON_CYCLE_MESSAGE : DEFAULT_MESSAGE);
}

void InputController::SetHighsAndLows()
{
  for (int i = 0; i < sizeof(lows) / sizeof(lows[0]); i++)
  {
    pinMode(lows[i], OUTPUT);
    digitalWrite(lows[i], LOW);
  }

  for (int i = 0; i < sizeof(highs) / sizeof(highs[0]); i++)
  {
    pinMode(highs[i], OUTPUT);
    digitalWrite(highs[i], HIGH);
  }
}
