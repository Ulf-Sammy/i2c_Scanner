// --------------------------------------
// i2c_scanner und i2c Tester
//
// 
//
//
#include <Wire.h>
#define MaxGruppe 8
#define MaxMelder MaxGruppe*8

struct Melder_Info
{
    bool Update;
    byte Data;
    byte oldData;
};

byte AnzahlGruppe;       // Anzahl MelderGruppe
bool AdressenI2C[127];
byte GruppeAddr[MaxGruppe];
volatile Melder_Info GruppeData[MaxGruppe];


void setup()
{
    Wire.begin();

    Serial.begin(115200);
    while (!Serial);            
    Serial.println("\nI2C Scanner");
    Serial.println("Scanning ");
    InitMelder();
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
    for (int m = 0; m < AnzahlGruppe; m++)
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
    int MelderNr;
    for (int m = 0; m < AnzahlGruppe; m++)
    {
        Wire.requestFrom((int)GruppeAddr[m], (int)1);
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
  
    AnzahlGruppe = 0;
    AdressenI2C[0] = false;
    Wire.begin();
    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0)
        {
            GruppeAddr[AnzahlGruppe] = address;
            AdressenI2C[address] = true;
            AnzahlGruppe++;
        }
        else
        {
            AdressenI2C[address] = false;
        }
        Serial.print(".");
    }
    Serial.println(" ");
    Serial.print("Found Device = ");
    Serial.println(AnzahlGruppe);
    for (int m = 0; m < AnzahlGruppe; m++)
    {
        Wire.requestFrom((int)GruppeAddr[m], (int)1);
        if (Wire.available())
        {
            GruppeData[m].Data = (0xFF ^ Wire.read());
            GruppeData[m].oldData = GruppeData[m].Data;
            GruppeData[m].Update = false;
        }
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
            Serial.print(" = 0x"); Serial.println(GruppeAddr[m], HEX);
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