#include "AT24C.h"
#include "Lib595.h"
extern IO595 *p595;
extern EEPROM_AT24C *pEEPROM;
unsigned char data[32 * 255] = {0};
unsigned char data_read[32 * 255] = {0};
void setup()
{
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    begin_595(p595, 12, 13, 2, 4);
    ESP.wdtDisable();
    pinMode(2, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(14, OUTPUT_OPEN_DRAIN);
    pinMode(0, OUTPUT);
    begin_AT24C(pEEPROM, AT24C64, 0, 14, PIN_595_BEGIN + 6, 0);
    Serial.begin(115200);
    Serial.println("EEPROM test begin");
}

void loop()
{
    static int t = 0;
    t++;
    Serial.printf("writing...\r\n");
    for (int i = 0; i < 255; ++i)
    {
        for (int j = 0; j < 32; ++j)
        {

            data[i * 32 + j] = t + i + j;
        }
    }
    write_595(p595, 0, LOW);
    release_WP_AT24C(pEEPROM);
    write_array_AT24C(pEEPROM, 1, data, 32 * 16);
    WP_AT24C(pEEPROM);
    Serial.printf("done writing\r\n");
    write_595(p595, 0, HIGH);
    delay(1000);
    Serial.printf("reading page %d\r\n");
    //read_array_AT24C(pEEPROM, PAGE2ADDR(pEEPROM, 1), data_read, 32*255);
    unsigned char d = read_AT24C(pEEPROM, PAGE2ADDR(pEEPROM, 1));
    //Serial.println(d);
    Serial.printf("PAGE %d: %d\r\n", 1, d);
    delay(1000);
}
