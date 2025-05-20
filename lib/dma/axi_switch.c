#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "axi_switch.h"

int switch_config(uint32_t *switch_reg_p)
{
	switch_reg_p[XIL_SWITCH_MI_MUX0] = XIL_SWITCH_DISABLE;
	switch_reg_p[XIL_SWITCH_MI_MUX1] = XIL_SWITCH_DISABLE;
	switch_reg_p[XIL_SWITCH_MI_MUX2] = XIL_SWITCH_DISABLE;
	switch_reg_p[XIL_SWITCH_CTRL] = 0x02;
	
	switch_reg_p[XIL_SWITCH_MI_MUX0] = 0x02;	// usb_dma_mm2s	=> m_axis_usb0
	switch_reg_p[XIL_SWITCH_MI_MUX1] = 0x01;	// s_axis_usb1	=> m_axis_usb1
	switch_reg_p[XIL_SWITCH_MI_MUX2] = 0x00;	// s_axis_usb0	=> usb_dma_s2mm
	
	// implying switch regs
	switch_reg_p[XIL_SWITCH_CTRL] = 0x02;

	return 1;
}
