#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Keypad.h>

#define mySerial Serial1
// Define the fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

// Define the relay, buzzer and servo pins
const int relayPin = 35;
const int servoPin = 34;
const int buzzer = A7;
const int redLED = 24;
const int greenLED =25;

// Define the maximum number of enrolled fingerprints
const int maxFingerprints = 100;

// An array to store enrolled fingerprint IDs
uint8_t id;
uint8_t enrolledIDs[maxFingerprints];
int numEnrolled = 0;

// Define the keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte rowPins[ROWS] = { 4, 3, 2, 49 };// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 48, 7, 6, 5 };

Keypad keypad = Keypad( makeKeymap(keys),  rowPins, colPins, ROWS, COLS);

// Define the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);


String userInput = "";
bool inputCompleted = false;
bool fingerDetected;
Servo doorServo;

struct User {
  uint16_t id;
  uint8_t fingerID;
};

User users[maxFingerprints]; // Array to store enrolled users

void setup() {
  // Initialize the fingerprint sensor 
  Serial.begin(9600);
  finger.begin(57600);
  if (finger.verifyPassword()) {
    //Serial.println("Fingerprint sensor found!");
  } else {
    //Serial.println("Fingerprint sensor not found. Check wiring.");
    while (1);
  }

  // Initialize the relay and servo
  pinMode(relayPin, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  doorServo.attach(servoPin);
  doorServo.write(0); // Initial servo position (closed)

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1.Reg  2.!Reg");

}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() 
{
  
  char key = keypad.getKey();

  if (key) 
  {
    if (key == '#') {
      inputCompleted = true;
    } else {
      userInput += key;
      lcd.setCursor(0, 1);
      lcd.print(userInput);
    }
  }

  if (inputCompleted) 
  {
    // Perform the action based on the entered number using a switch statement
    int number = userInput.toInt();

    switch (number) 
    {
      case 1:
        // Ask the user to scan their fingerprint
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Place your finger");
        lcd.setCursor(0, 1);
        lcd.print("on the sensor");

        delay(5000);
        
        checkFingerprint(); 
        
        break;
    
      case 2:
        getFingerprintEnroll();
        break;

      default:
        lcd.clear();
        lcd.print("Invalid Option");
        break;
    }

    // Clear the input for the next entry
    userInput = "";
    inputCompleted = false;
    delay(2000); // Display the result for 2 seconds before clearing
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1.Reg  2.!Reg");
  }

  
}


int checkFingerprint() 
{
  //Serial.println("Place your finger on the sensor...");

  int result = finger.getImage();
  if (result != FINGERPRINT_OK) {
    Serial.println("Failed to get fingerprint image");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed to scan.");
    return;
  }

  result = finger.image2Tz(1);
  if (result != FINGERPRINT_OK) {
    Serial.println("Failed to convert image");
    return;
  }

  uint16_t id = 0;
  result = finger.fingerFastSearch();
  if (result == FINGERPRINT_OK) {
    id = finger.fingerID;
    //Serial.print("Found fingerprint with ID #"); 
    //Serial.println(id);

    Serial.print("M");
    Serial.println(id);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter OTP");
    

    String otp = getOTP();
    String rOtp = rOTP();

   

    if(otp == rOtp)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WELCOME");
      digitalWrite( buzzer, HIGH);
      delay(500);
      digitalWrite(buzzer, LOW);
      delay(500);
      digitalWrite( buzzer, HIGH);
      delay(500);
      digitalWrite(buzzer, LOW);
      delay(500);
      openDoor();
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wrong OPT");
      lcd.setCursor(0, 1);
      lcd.print("Try again.");
      digitalWrite(redLED, HIGH);
      delay(2000);
      digitalWrite(redLED, LOW);

    }
  } 
  else 
  {
    Serial.println("No match found");
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("No match found");
    delay(2000);
    return;
  }

  // Check if the scanned fingerprint matches an enrolled user
  for (int i = 0; i < sizeof(users) / sizeof(users[0]); i++) {
    if (id == users[i].fingerID) {
      
      
    }
  }
  return finger.fingerID;
}

uint8_t getFingerprintEnroll() {

  if (numEnrolled >= maxFingerprints) 
  {
    Serial.println("Maximum number of fingerprints enrolled.");
    return;
  }

  char idChar = '\0';
  while (idChar == '\0') {
    char key = keypad.getKey();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter a an ID.");
    lcd.setCursor(0, 1);
    lcd.print(key);

    if (key && key >= '1' && key <= '9') {
      idChar = key;
      break;
    } else if (key == '#') {
      Serial.println("Invalid ID. Use a non-zero ID (1-9) and press '#' to enroll.");
    }
  }

  int id = idChar - '0';


  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); 
  Serial.println(id);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enrolling ID: ");
  lcd.setCursor(15, 0);
  lcd.print(id);

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprints");
    lcd.setCursor(0, 1);
    lcd.print("Don't match");

    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    users[numEnrolled].id = numEnrolled++;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Registered");
    lcd.setCursor(0, 1);
    lcd.print("Successfully.");

    Serial.print("Fingerprint enrolled with ID ");
    Serial.println(id);
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}


String getOTP() {
  String enteredOTP = "";
  char key = keypad.getKey();
  
  while (key != '#') {
    if (key) {
      lcd.setCursor(enteredOTP.length(), 1);
      lcd.print('*');
      enteredOTP += key;
    }
    key = keypad.getKey();
  }
  
  return enteredOTP;
}


String rOTP()
{
  String rcvotp = "";
  if (Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    rcvotp += input;

  }

  return rcvotp;
}

int getUserID()
{
  String idStr = "";
  while (true) 
  {
    char key = keypad.getKey();
    if (key) 
    {
      if (key == '#') 
      {
        break; // End of ID entry
      } 
      else 
      {
        idStr += key;
        lcd.print(key);
      }
    }
  }
  return idStr.toInt();
}

void openDoor() 
{
  // Activate the relay and open the door using the servo
  digitalWrite(greenLED, HIGH);
  digitalWrite(relayPin, HIGH);
  doorServo.write(100); // Adjust the servo angle for your setup
  delay(5000); // Wait for the door to open
  digitalWrite(greenLED, LOW);
  digitalWrite(relayPin, LOW);
  doorServo.write(0); // Close the door
}


