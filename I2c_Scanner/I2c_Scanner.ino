// --------------------------------------
// i2c_scanner und i2c Tester
//
// 
//
//
#include <Wire.h> 
#include <LiquidCrystal.h>

#define KEYPAD_KEY_RIGHT  0
#define KEYPAD_KEY_UP     1
#define KEYPAD_KEY_DOWN   2
#define KEYPAD_KEY_LEFT   3
#define KEYPAD_KEY_SELECT 4
#define KEYPAD_KEY_NONE   5

#define KEYPAD_KEY_RIGHT_ADC_LOW   0
#define KEYPAD_KEY_RIGHT_ADC_HIGH  20
#define KEYPAD_KEY_UP_ADC_LOW      120
#define KEYPAD_KEY_UP_ADC_HIGH     140
#define KEYPAD_KEY_DOWN_ADC_LOW    290
#define KEYPAD_KEY_DOWN_ADC_HIGH   339
#define KEYPAD_KEY_LEFT_ADC_LOW    450
#define KEYPAD_KEY_LEFT_ADC_HIGH   520
#define KEYPAD_KEY_SELECT_ADC_LOW  700
#define KEYPAD_KEY_SELECT_ADC_HIGH 780
#define KEYPAD_KEY_NONE_ADC_LOW    1000
#define KEYPAD_KEY_NONE_ADC_HIGH   102

#define PIN_KEY A0
#define PIN_BACKLIGHT 10
#define MaxGruppe 8

struct Melder_Info
{
    bool Update;
    byte Data;
    byte oldData;
};

byte AnzahlGruppeG;       // Anzahl MelderGruppe
byte AnzahlGruppeT;       // Anzahl MelderGruppe
bool AdressenI2C[127];
byte GruppeAddr_G[MaxGruppe];
byte GruppeAddr_T[MaxGruppe];


volatile Melder_Info GruppeData[MaxGruppe];
LiquidCrystal LCD(8, 9, 4, 5, 6, 7);


void setup()
{
    pinMode(PIN_BACKLIGHT, INPUT);
 //   digitalWrite(PIN_BACKLIGHT, 0);
    Wire.begin();
    LCD.begin(16, 2);
    LCD.clear();

    LCD.setCursor(0, 0);
    LCD.print("I2C Scanner V2.0");
    Serial.begin(115200);
    Serial.println("\nI2C Scanner V2.0");
    Serial.println("Scanning ....");
    LCD.setCursor(0, 1);
    LCD.print("Scanning ....");
    InitMelder();
    Show_LCD_MelderG();
    Serial.println("showing Data...");

    Serial.println("=====================================================");
   

    for (byte address = 1; address < 127; address++)
    {
        if(AdressenI2C[address])
        {
            Serial.print("ox");
            Serial.print(address, HEX);
        }
        else
        {
            Serial.print("_   ");
        }
        if ((address % 8) == 0) Serial.println(" |");
        else             Serial.print("  ");

    }
    Serial.println("           |");
    Serial.println("=====================================================");
}


void loop()
{
    ReadMelder();
    for (int m = 0; m < AnzahlGruppeG; m++)
    {
        if (GruppeData[m].Update)
        {
            printbitMuster(m);
            GruppeData[m].Update = false;
        }
    }
}

void ReadMelder()
{
    byte MelderData;
    for (int m = 0; m < AnzahlGruppeG; m++)
    {
        Wire.requestFrom((int)GruppeAddr_G[m], (int)1);
        if (Wire.available())
        {
            MelderData = (0xFF ^ Wire.read());
            if (GruppeData[m].Data != MelderData)
            {
                GruppeData[m].Data = MelderData;
                GruppeData[m].Update = true;
            }
        }
    }
}

void InitMelder()
{
    byte error;
  
    AnzahlGruppeG = 0;
    AnzahlGruppeT = 0;
    AdressenI2C[0] = false;

    Wire.begin();
    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0)
        {
            if ((address >= 0x20) && (address <= 0x27))
            {
                GruppeAddr_T[AnzahlGruppeT] = address;
                AnzahlGruppeT++;
            }
            if ((address >= 0x38) && (address <= 0x3F))
            {
                GruppeAddr_G[AnzahlGruppeG] = address;
                AnzahlGruppeG++;
                if(AnzahlGruppeG == 4)     LCD.setCursor(0, 1);
            }
            AdressenI2C[address] = true;
        }
        else
        {
            AdressenI2C[address] = false;
        }
        Serial.print(".");
    }
    Serial.println(" ");
    Serial.print("Found Device = ");
    Serial.println(AnzahlGruppeG);
    for (int m = 0; m < AnzahlGruppeG; m++)
    {
        Wire.requestFrom((int)GruppeAddr_G[m], (int)1);
        if (Wire.available())
        {
            GruppeData[m].Data = (0xFF ^ Wire.read());
            GruppeData[m].oldData = GruppeData[m].Data;
            GruppeData[m].Update = false;
        }
    }
    Serial.print("Read Melder ...");
    Serial.println(AnzahlGruppeG);
}

void Show_LCD_MelderG()
{
    byte V;
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("Read Melder = ");
    LCD.setCursor(14, 0);
    LCD.print(AnzahlGruppeG);
    LCD.setCursor(0, 1);
    LCD.print("x3:");
    for (int m = 0; m < AnzahlGruppeG; m++)
    {
        V = GruppeAddr_G[m] & 0x0F;
        LCD.print(" ");
        LCD.print(V,HEX);
    }

}

void printbitMuster(byte m)
{
    static byte lastByte = 0xFF;
    static bool lastrun = false;
    if (lastrun)
    {
        PrintByteString(GruppeData[m].Data, GruppeData[m].oldData);
        Serial.println("_________");
        GruppeData[m].oldData = GruppeData[m].Data;
        lastrun = false;
    }
    else
    {
        if ((GruppeData[m].Data != lastByte)&& (GruppeData[m].Data != 0 ) )
        {
            lastByte = GruppeData[m].Data;
            lastrun = true;
            PrintByteString(GruppeData[m].Data, GruppeData[m].oldData);
            Serial.print(" = 0x"); Serial.println(GruppeAddr_G[m], HEX);
            GruppeData[m].oldData = GruppeData[m].Data;
        }
    }
}

void PrintByteString(byte A, byte B)
{
    byte  Mask = 0x01;
    bool Bit1;
    bool Bit2;
    if (A != B)
    {
        for (int i = 0; i < 8; i++)
        {
            Bit1 = A & Mask;
            Bit2 = B & Mask;
            if (Bit1 == Bit2)
                Serial.print("-");
            else
            {
                if (Bit1)  Serial.print("I");
                else       Serial.print("O");
            }
            Mask = Mask << 1;
        }
        Serial.print(" |");
    }
}
byte read_key(void)
{
    uint16_t adcval = analogRead(PIN_KEY);

    if ((adcval >= KEYPAD_KEY_RIGHT_ADC_LOW) && (adcval <= KEYPAD_KEY_RIGHT_ADC_HIGH))   return KEYPAD_KEY_RIGHT;
    if ((adcval >= KEYPAD_KEY_UP_ADC_LOW) && (adcval <= KEYPAD_KEY_UP_ADC_HIGH))         return KEYPAD_KEY_UP;
    if ((adcval >= KEYPAD_KEY_DOWN_ADC_LOW) && (adcval <= KEYPAD_KEY_DOWN_ADC_HIGH))     return KEYPAD_KEY_DOWN;
    if ((adcval >= KEYPAD_KEY_LEFT_ADC_LOW) && (adcval <= KEYPAD_KEY_LEFT_ADC_HIGH))     return KEYPAD_KEY_LEFT;
    if ((adcval >= KEYPAD_KEY_SELECT_ADC_LOW) && (adcval <= KEYPAD_KEY_SELECT_ADC_HIGH)) return KEYPAD_KEY_SELECT;
    if ((adcval >= KEYPAD_KEY_NONE_ADC_LOW) && (adcval <= KEYPAD_KEY_NONE_ADC_HIGH))     return KEYPAD_KEY_NONE;
    return KEYPAD_KEY_NONE;
}