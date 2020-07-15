#include "HD44780_824.h"
#include "mrt.h"

#define set_en P1OUT |= EN
#define clr_en P1OUT &= ~EN

uint8_t InitLCD(void)
{
    uint8_t errCode;

    INIT_BUS

    DelayMs(64);
    DelayMs(36);

    P3OUT |= DB5_PIN | DB4_PIN; // DB7=0, DB6=0, DB5=1, DB4=1, NU=0, EN=0, RW=0, RS=0
    __delay_cycles(1);

    set_en;
    __delay_cycles(1);
    clr_en;
    DelayMs(5);

    set_en;
    __delay_cycles(1);
    clr_en;
    DelayUs(200);

    set_en;
    __delay_cycles(1);
    clr_en;
    DelayUs(100);

    P3OUT |= DB5_PIN; // DB7=0, DB6=0, DB5=1, DB4=0, NU=0, EN=0, RW=0, RS=0
    P3OUT &= ~DB4_PIN;
    __delay_cycles(1);

    set_en;
    __delay_cycles(1);
    clr_en;
    DelayMs(5);

//  errCode = WriteByte(SEL_IR, FOUR_BIT_ONE_LINE_5x10); //Define function set 8 bit interface, 1-line line display, 5x10 font
    errCode = WriteByte(SEL_IR, FOUR_BIT_TWO_LINE_5x8);  //Define function set 8 bit interface, 1-line line display, 5x8 font
    DelayMs(1);                                          // Wait until LCD is free

    errCode = WriteByte(SEL_IR, DISP_ON_CUR_ON_BLINK_OFF); // Define display on/off control
    DelayMs(1);                                         // Wait until LCD is free

    errCode = WriteByte(SEL_IR, DISPLAY_CLEAR);             // Clear display
    DelayMs(1);                                         // Wait until LCD is free

    errCode = WriteByte(SEL_IR, ENTRY_MODE_INC_NO_SHIFT);   // Entry mode set cursor direction increment, do not shift display
    DelayMs(10);                                            // Wait until LCD is free

    PutCommand(DISPLAY_CLEAR); DelayMs(5);
    PutCommand(RETURN_HOME);   DelayMs(5);

    return errCode;
}

/**************************************
*****     Write a byte on LCD      ****
***************************************/
uint8_t WriteByte(uint8_t rs, uint8_t data_to_LCD)
{
    uint32_t dataVal;

    if(rs)
        P1OUT |= RS;     // Set RS (DR operation)
    else
        P1OUT &= ~RS;     // Clear RS (IR operation)

    __delay_cycles(1);      //DelayUs(1);                         // Wait a minimum of 60ns

    P3OUT &= ~DATA_BUS;
    set_en;         // Set EN in order to start first write operation

    dataVal = (data_to_LCD & 0xF0)>>4;  // Copy upper nibble into dataVal
    dataVal = AlignDataPin(dataVal);    // align DB7:DB4 on DATA_BUS

    P3OUT |= dataVal; // data bus reflects upper nibble
    __delay_cycles(1); //    DelayUs(1);                         // Wait a minimum of 195ns

    clr_en;         // Clear EN finish first write operation
    __delay_cycles(1); //DelayUs(1);                         // Wait a minimum of 530ns between writes

    P3OUT &= ~DATA_BUS;
    set_en;         // Raise EN start second write operation

    dataVal = data_to_LCD & 0x0F;       // Copy lower nibble into dataVal
    dataVal = AlignDataPin(dataVal);    // align DB7:DB4 on DATA_BUS

    P3OUT |= dataVal; // data bus reflects lower nibble
    __delay_cycles(1);  //DelayUs(1);                         // Wait a minimum of 195ns

    clr_en;         // Clear EN finish second write operation
    __delay_cycles(1); // DelayUs(1);                         // Wait a minimum of 30ns
    P3OUT &= ~DATA_BUS;

    return 0;
}

/***************************************
***** Align 4 bits data to Data BUS ****
****************************************/
uint32_t AlignDataPin(uint8_t data)
{
    uint32_t datapin = 0;

    if(data & BIT0)
        datapin |= DB4_PIN;
    if(data & BIT1)
        datapin |= DB5_PIN;
    if(data & BIT2)
        datapin |= DB6_PIN;
    if(data & BIT3)
        datapin |= DB7_PIN;

    return datapin;
}

/**************************************
*****    Send a command to LCD     ****
***************************************/
uint8_t PutCommand(uint8_t Command)
{
    DelayMs(1);                         // Wait until LCD is free
    return WriteByte(SEL_IR, Command);      // Write character to IR
}

/*-----------------------------------------------------
 | This function cannot be applied to DM0810 (1 line) |
 -----------------------------------------------------*/
uint8_t GoToLine(uint8_t line)
{
    uint8_t address;
    switch(line)
    {
        case 1:
            address = 0x00;
        break;
        case 2:
            address = 0x40;
        break;
        case 3:
            address = 0x10;
        break;
        case 4:
            address = 0x50;
        break;
        default:
        address = 0x00;
        break;
    }
    return PutCommand(DDRAM_ADDRESS(address));
}

/**************************************
*****     Write 2 digit on LCD     ****
*****      with leading zeros      ****
***************************************/
void Write_2digitsval(uint32_t dummyVal)
{
    WriteChar(NUM_TO_CODE(dummyVal/10));
    dummyVal %= 10;
    WriteChar(NUM_TO_CODE(dummyVal));
}

/**************************************
*****     Write n digit on LCD     ****
*****     without leading zeros    ****
***************************************/
void Write_ndigitsval(uint32_t dummyVal, uint8_t ndigits)
{
    uint32_t ten_base=1, digit, leading_zeroes_flag = 1;

    while(--ndigits)
        ten_base*=10;

    while(ten_base)
    {
        digit = dummyVal/ten_base;
        if(digit)
            leading_zeroes_flag = 0;
        WriteChar(leading_zeroes_flag&&(ten_base>1)? ' ': NUM_TO_CODE(digit));
        dummyVal %= ten_base;
        ten_base/=10;
    }
}

/**************************************
*****   Write a character on LCD   ****
***************************************/
uint8_t WriteChar(uint8_t character)
{
    DelayMs(1);                         // Wait until LCD is free
    return WriteByte(SEL_DR, character);    // Write character to DR
}

/**************************************
*****    Write a string on LCD     ****
*****      starting at home        ****
***************************************/
void WriteString(uint8_t LineOfCharacters[TOTAL_CHARACTERS_OF_LCD], uint8_t line)
{
    uint8_t i=0 /*, errCode, line=0*/;

    GoToLine(line);
    while(LineOfCharacters[i] && i<TOTAL_CHARACTERS_OF_LCD)
    {
        if((i%LCD_LINE_LENGHT)==0)
            GoToLine(++line);

        /*errCode = */WriteChar(LineOfCharacters[i]);
        i++;
    }
}


void WriteLine(uint8_t lineOfCharacters[LCD_LINE_LENGHT], uint8_t line)
{
    uint8_t i=0;

    if(line!=0)
        GoToLine(line);

    while(lineOfCharacters[i] && i<LCD_LINE_LENGHT)
    {
        WriteChar(lineOfCharacters[i]);
        i++;
    }
}

/**************************************
*****    Write a string on LCD     ****
***** starting from cursor position****
***************************************/
void WriteWord(uint8_t LineOfCharacters[TOTAL_CHARACTERS_OF_LCD])
{
    uint8_t i = 0;
    while(LineOfCharacters[i] && i<LCD_LINE_LENGHT)
    {
        WriteChar(LineOfCharacters[i]);
        i++;
    }
}

// EOF
