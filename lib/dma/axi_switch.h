
#define XIL_SWITCH_CTRL					0x00/4
#define XIL_SWITCH_MI_MUX0				0x40/4
#define XIL_SWITCH_MI_MUX1				0x44/4
#define XIL_SWITCH_MI_MUX2				0x48/4
#define XIL_SWITCH_DISABLE				0x80000000

int switch_config(uint32_t *switch_reg_p);
