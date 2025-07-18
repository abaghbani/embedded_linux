/*! 
	@file examples/st7789/Hello_world/main.cpp
	@brief Hello World hardware SPI 0 test
	@author Gavin Lyons.
	@note See USER OPTIONS 1-3 in SETUP function, run as sudo
		Tests hardware SPI 0
	@test 
		-# 101 Hello World Hardware SPI
*/

// Section ::  libraries 
#include <bcm2835.h> 
#include "ST7789_TFT_LCD_RVL.hpp"

// Section :: Globals 
ST7789_TFT myTFT;
// Display size in pixels
#define myTFTWidth 240
#define myTFTHeight 280

//  Section ::  Function Headers 

uint8_t SetupHWSPI(void); // setup + user options for hardware SPI 0
void HelloWorld(void);
void EndTests(void);

//  Section ::  MAIN loop

int main() 
{

	if(SetupHWSPI() != 0) return -1; //Hardware SPI 0
	HelloWorld();
	EndTests();
	return 0;
}
// *** End OF MAIN **


//  Section ::  Function Space 

// Hardware SPI setup
uint8_t SetupHWSPI(void)
{
	std::cout << "TFT Start Test 101 HW SPI 0" << std::endl;
	if(!bcm2835_init())
	{
		std::cout << "Error 1201 Problem with init bcm2835 library" << std::endl;
		return 2;
	}

// ** USER OPTION 1 GPIO HW SPI **
	int8_t RST_TFT  = 13;
	int8_t DC_TFT   = 25;
	myTFT.TFTSetupGPIO(RST_TFT, DC_TFT);
//*********************************************

// ** USER OPTION 2 Screen SetupHWSPI **
	uint16_t OFFSET_COL = 0;  // These offsets can be adjusted for any issues->
	uint16_t OFFSET_ROW = 0;  // with manufacture tolerance/defects at edge of display
	uint16_t TFT_WIDTH = myTFTWidth;   // Screen width in pixels
	uint16_t TFT_HEIGHT = myTFTHeight; // Screen height in pixels
	myTFT.TFTInitScreenSize(OFFSET_COL, OFFSET_ROW , TFT_WIDTH , TFT_HEIGHT);
// ***********************************

// ** USER OPTION 3 SPI baud rate + SPI_CE_PIN**
	uint32_t SCLK_FREQ =  8000000 ; //  freq in Hertz , MAX 125 Mhz MIN 30Khz
	uint8_t SPI_CE_PIN = 0; // which HW SPI chip enable pin to use,  0 or 1
	if(myTFT.TFTInitSPI(SCLK_FREQ, SPI_CE_PIN) != rvlDisplay_Success)
	{
		bcm2835_close(); //Close lib & /dev/mem, deallocating mem
		return 3;
	}
//*****************************
	std::cout << "ST7789 library version : " << GetRVLibVersionNum()<< std::endl;
	std::cout <<"bcm2835 library version :" << bcm2835_version() << std::endl;
	delayMilliSecRVL(100);
	return 0;
}

void HelloWorld(void) 
{
	std::cout << "Hello World" << std::endl;
	myTFT.TFTsetRotation(myTFT.TFT_Degrees_90); // Rotate the display
	myTFT.fillScreen(RVLC_BLACK);
	myTFT.setCursor(42,60);
	myTFT.setTextColor(RVLC_GREEN, RVLC_BLACK);
	myTFT.setFont(font_orla);
	myTFT.print("Hello Baghbani's");
	myTFT.print("   Family");
	delayMilliSecRVL(7000);
}

void EndTests(void)
{
	myTFT.fillScreen(RVLC_BLACK);
	myTFT.TFTPowerDown(); // Power down device
	bcm2835_close(); // Close the bcm2835 library
	std::cout << "TFT End" << std::endl;
}

// *************** EOF ****************
