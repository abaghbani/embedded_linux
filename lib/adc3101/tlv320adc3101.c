#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "i2c.h"

void tlv320adc3101_init(struct I2cDevice* dev, uint32_t sample_rate, uint8_t sample_width) 
{
    // Page select
    i2c_write_reg(dev, 0x00, 0x00); // Page 0
    
    // Reset device
    i2c_write_reg(dev, 0x01, 0x01); // Software reset

    // Use PLL, MCLK as source
    i2c_write_reg(dev, 0x04, 0x03);
    
    if (sample_rate == 48000) {
        // PLL config for 48kHz, MCLK = 16MHz, PLL = 96MHz
        // PLL_CLK = 16MHz × 6 = 96MHz
        // ADC_CLK = 96MHz / (2×2) = 24MHz
        // ADC_D_CLK = 24MHz / 128 = 48kHz
        i2c_write_reg(dev, 0x05, 0x91); // PLL P=1, R=1
        i2c_write_reg(dev, 0x06, 0x06); // J = 6
        i2c_write_reg(dev, 0x07, 0x00); // D = 0 (MSB)
        i2c_write_reg(dev, 0x08, 0x00); // D = 0 (LSB)
    
        // NDAC and MDAC
        i2c_write_reg(dev, 0x12, 0x82); // NDAC = 2
        i2c_write_reg(dev, 0x13, 0x82); // MDAC = 2
    
        // DOSR = 128
        i2c_write_reg(dev, 0x14, 0x80);
        i2c_write_reg(dev, 0x15, 0x00);
    } else if (sample_rate == 44100) {
        // PLL config for 44.1kHz, MCLK = 16MHz, PLL = 114.0736MHz
        // PLL_CLK = (16MHz × (7 + 0.128)) ≈ 114.0736 MHz
        // ADC_CLK = 114.0736 / 4 = 28.5184 MHz
        // ADC_D_CLK = 28.5184 / 128 ≈ 44.1kHz
        i2c_write_reg(dev, 0x05, 0x91); // PLL P=1, R=1
        i2c_write_reg(dev, 0x06, 0x07); // J = 7
        i2c_write_reg(dev, 0x07, 0x14); // D = 0x1490 (MSB)
        i2c_write_reg(dev, 0x08, 0x90); // D = 0x1490 (LSB)
    
        // NDAC and MDAC
        i2c_write_reg(dev, 0x12, 0x82); // NDAC = 2
        i2c_write_reg(dev, 0x13, 0x82); // MDAC = 2
    
        // DOSR = 128
        i2c_write_reg(dev, 0x14, 0x80);
        i2c_write_reg(dev, 0x15, 0x00);
    }
    // Codec Interface & Audio Format
    i2c_write_reg(dev, 0x1B, 0x00); // I2S mode, slave

    if (sample_width == 16)
        i2c_write_reg(dev, 0x1C, 0x00); // 16-bit word length
    else if (sample_width == 24)
        i2c_write_reg(dev, 0x1C, 0x20); // 24-bit word length

    // Route Line inputs IN1_L and IN1_R to ADC
    i2c_write_reg(dev, 0x34, 0x30); // IN1_L to LADC_P
    i2c_write_reg(dev, 0x36, 0x30); // IN1_R to RADC_P
    i2c_write_reg(dev, 0x37, 0x00); // CM to LADC_M
    i2c_write_reg(dev, 0x39, 0x00); // CM to RADC_M

    // Power up MIC bias (if needed)
    // i2c_write_reg(dev, 0x33, 0x40); // Optional

    // Power up ADCs
    i2c_write_reg(dev, 0x51, 0xC0); // Power up LADC and RADC

    // Unmute ADCs
    i2c_write_reg(dev, 0x52, 0x00); // Unmute LADC
    i2c_write_reg(dev, 0x53, 0x00); // Unmute RADC

    // Route ADC to I2S output
    i2c_write_reg(dev, 0x3D, 0x00); // Left ADC to left DAC
    i2c_write_reg(dev, 0x3E, 0x00); // Right ADC to right DAC

    // Power up digital interface
    i2c_write_reg(dev, 0x00, 0x00); // Page 0 again
    i2c_write_reg(dev, 0x3F, 0xD4); // DAC => I2S
}

void tlv320adc3101_set_led(struct I2cDevice* dev, uint8_t led1, uint8_t led2) 
{
    uint8_t reg_value = (led1 ? 0x40 : 0x00)+ (led2 ? 0x10 : 0x00);
    i2c_write_reg(dev, 0x00, 0x01); // Page 1
    i2c_write_reg(dev, 0x33, reg_value);
}
