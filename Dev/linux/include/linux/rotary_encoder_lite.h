#ifndef __ROTARY_ENCODER_LITE_H__
#define __ROTARY_ENCODER_LITE_H__

struct rotary_encoder_lite_platform_data {
	unsigned int gpio_a;
	unsigned int gpio_b;
	unsigned int gpio_clear;
	unsigned int steps;
	bool rollover;
};

#endif /* __ROTARY_ENCODER_LITE_H__ */
