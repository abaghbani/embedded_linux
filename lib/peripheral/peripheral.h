#include "i2c.h"
#include "spi.h"

#define linux_i2c_device					"/dev/i2c-0"
#define linux_spi_device					"/dev/spidev1.1"
#define i2c_address_front_panel				0x41
#define i2c_address_system_controller		0x23

#define led_operating_dimming_address		0x10
#define led_activity_dimming_address		0x14
#define led_operating_address				0x18
#define led_activity_address				0x19

#define att_write_value_command				0x00
#define att_address_tx0						0x00
#define att_address_rx0						0x01
#define att_address_rx1						0x02

enum led_color {
	red = 1,
	green = 2,
	blue = 4
};

enum att_value {
	att_value_0db = 0,
	att_value_0p25db = 1,
	att_value_0p5db = 2,
	att_value_1db = 4,
	att_value_2db = 8,
	att_value_4db = 0x10,
	att_value_8db = 0x20,
	att_value_16db = 0x40,
	att_value_31p75db = 0x7f
};

typedef enum { false, true } bool;

struct peripheral_info {
	struct I2cDevice i2c_device_p;
	struct SPIDevice spi_device_p;
	
	bool led_update;
	bool attenuator_update;
	
	enum led_color led_activity;
	enum led_color led_operating;
	enum att_value attenuator;
};

int peripheral_init(struct peripheral_info * info);
int peripheral_stop(struct peripheral_info * info);
int peripheral(struct peripheral_info * info);
