/*
 *  si1143.c - Linux kernel modules for ambient light sensor and proximity
 *
 *  Copyright (C) 2010 Nest Labs, Inc
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/si1143.h>

#define SI1143_DRV_NAME	"si1143"
#define DRIVER_VERSION		"1.0"

/*
 * Defines
 */

#define SI1143_PART_ID		0x00
#define SI1143_PART_ID_VAL  0x43

#define SI1143_REV_ID 0x01
#define SI1143_SEQ_ID 0x02
#define SI1143_INT_CFG 0x03
#define SI1143_IRQ_ENABLE 0x04
#define SI1143_IRQ_MODE1 0x05
#define SI1143_IRQ_MODE2 0x06
#define SI1143_HW_KEY 0x07
#define SI1143_MEAS_RATE 0x08
#define SI1143_ALS_RATE 0x09
#define SI1143_PS_RATE 0x0A
#define SI1143_ALS_LOW_TH 0x0B
#define SI1143_ALS_HIGH_TH 0x0D
#define SI1143_PS_LED21 0x0F
#define SI1143_PS_LED3 0x10
#define SI1143_PS1_TH 0x11
#define SI1143_PARAM_WR  0x17
#define SI1143_COMMAND 0x18
#define SI1143_RESPONSE 0x20
#define SI1143_IRQ_STATUS 0x21
#define SI1143_PS1_DATA0  0x26
#define SI1143_PS1_DATA1  0x27
#define SI1143_ALS_VIS_DATA0  0x22
#define SI1143_ALS_VIS_DATA1  0x23
#define SI1143_ALS_IR_DATA0  0x24
#define SI1143_ALS_IR_DATA1  0x25
#define SI1143_PARAM_RD 0x2E


//Prox/ALS commands
#define SI1143_PSALS_FORCE 0x07
#define SI1143_PS_FORCE 0x05
#define SI1143_ALS_FORCE 0x06

#define SI1143_PS_AUTO 0x0D

#define SI1143_PARAM_SET 0xA0
#define SI1143_PARAM_QUERY 0x80

//Prox/ALS parameters
#define SI1143_CHLIST 0x01
#define SI1143_PSLED12_SELECT 0x02
#define SI1143_PS_ADC_GAIN 0x0B
#define SI1143_ALS_VIS_ADC_GAIN 0x11

#define SI1143_TH_LOW 0
#define SI1143_TH_HIGH 1

#define SI1143_ALS_VIS 0
#define SI1143_ALS_IR 1


#define SI1143_DEFAULT_PS_ADC_GAIN 2
#define SI1143_MAX_PS_ADC_GAIN 4 // could be 5, but that might get unsafe and damage the part

#define SI1143_DEFAULT_ALS_VS_ADC_GAIN 3

#define SI1143_ALS_IE  0x01
#define SI1143_PS1_IE  0x04

#define SI1143_PS1_IM 0x30
#define SI1143_ALS_VIS_IM  0x01
#define SI1143_ALS_IR_IM  0x03
#define SI1143_ALS_IM_MASK  0x03

/*
 * Structs
 */
struct si1143_data {
	struct i2c_client *client;
	struct mutex update_lock;
	struct delayed_work	dwork;
	spinlock_t		lock;

	unsigned int power_state : 1;
	unsigned int operating_mode : 1;

	unsigned int als_interrupt_spectrum;

	unsigned int irq;

};

/*
 * Global data
 */


/*
 * Management functions
 */

static ssize_t __si1143_show_light(struct i2c_client *client, char *buf)
{
	u8 visiblelight[2];
	int err;

  	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_ALS_FORCE);
	if (err < 0)
		return err;

	mdelay(20);

	err = i2c_smbus_read_i2c_block_data(client, SI1143_ALS_VIS_DATA0,
					 2, visiblelight);

	if (err < 0)
		return err;
	
	return sprintf(buf, "%d\n", (visiblelight[1]<< 8) | visiblelight[0] );
}

static ssize_t si1143_show_light(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_show_light(client, buf);
	mutex_unlock(&data->update_lock);

	return ret;
}

static ssize_t __si1143_show_ir(struct i2c_client *client, char *buf)
{
	u8 infrared[2];
	int err;

  	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_ALS_FORCE);
	if (err < 0)
		return err;

	mdelay(20);

	// Read the PS1_DATA0
	err = i2c_smbus_read_i2c_block_data(client, SI1143_ALS_IR_DATA0,
					 2, infrared);
	if (err < 0)
		return err;
	
	return sprintf(buf, "%d\n", (infrared[1]<< 8) | infrared[0] );
}

static ssize_t si1143_show_ir(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_show_ir(client, buf);
	mutex_unlock(&data->update_lock);

	return ret;
}

static ssize_t __si1143_show_prox(struct i2c_client *client, char *buf)
{
	u8 proximity[2];
	int err;

	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PS_FORCE);
	if (err < 0)
		return err;

	mdelay(20);

	// Read the PS1_DATA0
	err = i2c_smbus_read_i2c_block_data(client, SI1143_PS1_DATA0,
					 2, proximity);
	if (err < 0)
		return err;

	return sprintf(buf, "%d\n", (proximity[1]<< 8) | proximity[0] );
}

static ssize_t si1143_show_prox(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_show_prox(client, buf);
	mutex_unlock(&data->update_lock);

	return ret;
}

static ssize_t __si1143_get_prox_th(struct i2c_client *client, char *buf)
{
	int err;
	u8 threshold;

	err = i2c_smbus_read_i2c_block_data(client, SI1143_PS1_TH,
					 1, &threshold);
	if (err < 0)
		return err;
	
	return sprintf(buf, "%d\n", (threshold << 8) );
}

static ssize_t si1143_get_prox_th(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_get_prox_th(client, buf);
	mutex_unlock(&data->update_lock);

	return ret;
}

static ssize_t __si1143_set_prox_th(struct i2c_client *client, u32 threshold)
{
	int err;
	u8 device_th = (u8) ((threshold & 0xFF00)>> 8);
	u8 interrupt_val=0;

	err = i2c_smbus_write_byte_data(client, SI1143_PS1_TH, device_th);
	if (err < 0)
		return err;

	err = i2c_smbus_read_i2c_block_data(client, SI1143_IRQ_ENABLE, 1, &interrupt_val);
	if (err < 0)
		return err;

	// Set the IRQ_ENABLE register
	err = i2c_smbus_write_byte_data(client, SI1143_IRQ_ENABLE, interrupt_val | SI1143_PS1_IE);
	if (err < 0)
		return err;
  
	err = i2c_smbus_read_i2c_block_data(client, SI1143_IRQ_MODE1, 1, &interrupt_val);
	if (err < 0)
		return err;

	// Set the IRQ_MODE1 register
	err = i2c_smbus_write_byte_data(client, SI1143_IRQ_MODE1, interrupt_val | SI1143_PS1_IM);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(client, SI1143_MEAS_RATE, 0xb9);//DF);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(client, SI1143_PS_RATE, 0x08);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PS_AUTO);
	if (err < 0)
		return err;
	else
		return 0;
}

static ssize_t si1143_set_prox_th(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t len)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;
	u32 threshold;

	sscanf(buf, "%u", &threshold);

	mutex_lock(&data->update_lock);
	ret = __si1143_set_prox_th(client, threshold);
	mutex_unlock(&data->update_lock);

	if(ret < 0)
	{
		return ret;
	}
	else
	{
		return strnlen(buf, PAGE_SIZE);
	}
}

static ssize_t __si1143_get_prox_gain(struct i2c_client *client, char *buf)
{
	int err;
	u8 gain;

	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PARAM_QUERY | SI1143_PS_ADC_GAIN);
	if (err < 0)
		return err;

	err = i2c_smbus_read_i2c_block_data(client, SI1143_PARAM_RD, 1, &gain);
	if (err < 0)
		return err;
	
	return sprintf(buf, "%d\n", gain );
}

static ssize_t si1143_get_prox_gain(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_get_prox_gain(client, buf);
	mutex_unlock(&data->update_lock);

	return ret;
}



static ssize_t __si1143_set_prox_gain(struct i2c_client *client, u8 gain)
{
	int err;

	err = i2c_smbus_write_byte_data(client, SI1143_PARAM_WR, min(gain, (u8) SI1143_MAX_PS_ADC_GAIN) );
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PARAM_SET | SI1143_PS_ADC_GAIN);
	if (err < 0)
		return err;
	else
		return 0;
}

static ssize_t si1143_set_prox_gain(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t len)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;
	u32 gain;

	sscanf(buf, "%u", &gain);

	mutex_lock(&data->update_lock);
	ret = __si1143_set_prox_gain(client, gain);
	mutex_unlock(&data->update_lock);

	if(ret < 0)
	{
		return ret;
	}
	else
	{
		return strnlen(buf, PAGE_SIZE);
	}
}

static ssize_t __si1143_get_als_spectrum(struct i2c_client *client, char *buf)
{
	int err;
	u8 interrupts;

	err = i2c_smbus_read_i2c_block_data(client, SI1143_IRQ_MODE1,
					 1, &interrupts);
	if (err < 0)
		return err;

	if ((interrupts & SI1143_ALS_IR_IM) == SI1143_ALS_IR_IM)
		return sprintf(buf, "infrared\n");
	else
		return sprintf(buf, "visible\n");
}

static ssize_t si1143_get_als_spectrum(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_get_als_spectrum(client, buf);
	mutex_unlock(&data->update_lock);

	return ret;
}



static ssize_t __si1143_set_als_spectrum(struct i2c_client *client, u32 visible)
{
	int err;
	u8 interrupt_val = 0;

	err = i2c_smbus_read_i2c_block_data(client, SI1143_IRQ_MODE1,
					 1, &interrupt_val);
	if (err < 0)
		return err;

	if (visible)
	{
		interrupt_val &= ~SI1143_ALS_IM_MASK;
		interrupt_val |= SI1143_ALS_VIS_IM;
	}
	else
	{
		interrupt_val &= ~SI1143_ALS_IM_MASK;
		interrupt_val |= SI1143_ALS_IR_IM;
	}

	err = i2c_smbus_write_byte_data(client, SI1143_IRQ_MODE1, interrupt_val);
	if (err < 0)
		return err;

	err = i2c_smbus_read_i2c_block_data(client, SI1143_IRQ_ENABLE, 1, &interrupt_val);
	if (err < 0)
		return err;

	// Set the IRQ_ENABLE register
	err = i2c_smbus_write_byte_data(client, SI1143_IRQ_ENABLE, interrupt_val | SI1143_ALS_IE);
	if (err < 0)
		return err;
	else
		return 0;
}

static ssize_t si1143_set_als_spectrum(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t len)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;
	u32 visible = 0;
	
	sscanf(buf, "%u", &visible);

	mutex_lock(&data->update_lock);
	ret = __si1143_set_als_spectrum(client, visible);
	mutex_unlock(&data->update_lock);

	if(ret < 0)
	{
		return ret;
	}
	else
	{
		return strnlen(buf, PAGE_SIZE);
	}
}

static ssize_t __si1143_get_als_th(struct i2c_client *client, char *buf, int limit)
{
	int err;
	u8 threshold;

	if(limit == SI1143_TH_HIGH)
		err = i2c_smbus_read_i2c_block_data(client, SI1143_ALS_HIGH_TH, 1, &threshold);
	else
		err = i2c_smbus_read_i2c_block_data(client, SI1143_ALS_LOW_TH, 1, &threshold);

	if (err < 0)
		return err;
	
	return sprintf(buf, "%d\n", (threshold << 8) );
}

static ssize_t si1143_get_als_th_high(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_get_als_th(client, buf, SI1143_TH_HIGH);
	mutex_unlock(&data->update_lock);

	return ret;
}

static ssize_t si1143_get_als_th_low(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = __si1143_get_als_th(client, buf, SI1143_TH_LOW);
	mutex_unlock(&data->update_lock);

	return ret;
}

static ssize_t __si1143_set_als_th(struct i2c_client *client, u32 threshold, int limit)
{
	int err;
	u8 device_th = (u8) ((threshold & 0xFF00)>> 8);

	if(limit == SI1143_TH_HIGH)
		err = i2c_smbus_write_byte_data(client, SI1143_ALS_HIGH_TH, device_th);
	else
		err = i2c_smbus_write_byte_data(client, SI1143_ALS_LOW_TH, device_th);

	if (err < 0)
		return err;
	else
		return 0;
}

static ssize_t si1143_set_als_th_high(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t len)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;
	u32 threshold;

	sscanf(buf, "%u", &threshold);

	mutex_lock(&data->update_lock);
	ret = __si1143_set_als_th(client, threshold, SI1143_TH_HIGH);
	mutex_unlock(&data->update_lock);

	if(ret < 0)
	{
		return ret;
	}
	else
	{
		return strnlen(buf, PAGE_SIZE);
	}
}

static ssize_t si1143_set_als_th_low(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t len)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct si1143_data *data = i2c_get_clientdata(client);
	int ret;
	u32 threshold;

	sscanf(buf, "%u", &threshold);

	mutex_lock(&data->update_lock);
	ret = __si1143_set_als_th(client, threshold, SI1143_TH_LOW);
	mutex_unlock(&data->update_lock);

	if(ret < 0)
	{
		return ret;
	}
	else
	{
		return strnlen(buf, PAGE_SIZE);
	}
}


static DEVICE_ATTR(proximity, S_IRUGO, si1143_show_prox, NULL);
static DEVICE_ATTR(illuminance_visible, S_IRUGO, si1143_show_light, NULL);
static DEVICE_ATTR(illuminance_infrared, S_IRUGO, si1143_show_ir, NULL);
static DEVICE_ATTR(proximity_threshold, (S_IRUGO | S_IWUGO ), si1143_get_prox_th, si1143_set_prox_th);
static DEVICE_ATTR(illuminance_threshold_high, (S_IRUGO | S_IWUGO ), si1143_get_als_th_high, si1143_set_als_th_high);
static DEVICE_ATTR(illuminance_threshold_low, (S_IRUGO | S_IWUGO ), si1143_get_als_th_low, si1143_set_als_th_low);
static DEVICE_ATTR(illuminance_interrupt_spectrum, (S_IRUGO | S_IWUGO ), si1143_get_als_spectrum, si1143_set_als_spectrum);
static DEVICE_ATTR(proximity_adc_gain, (S_IRUGO | S_IWUGO ), si1143_get_prox_gain, si1143_set_prox_gain);

static struct attribute *si1143_attributes[] = {
	&dev_attr_proximity.attr,
	&dev_attr_illuminance_visible.attr,
	&dev_attr_illuminance_infrared.attr,
	&dev_attr_proximity_threshold.attr,
	&dev_attr_illuminance_threshold_high.attr,
	&dev_attr_illuminance_threshold_low.attr,
	&dev_attr_illuminance_interrupt_spectrum.attr,
	&dev_attr_proximity_adc_gain.attr,
	NULL
};

static const struct attribute_group si1143_attr_group = {
	.attrs = si1143_attributes,
};

/*
 * Initialization function
 */

static int si1143_init_client(struct i2c_client *client, struct si1143_platform_data *platform_data)
{
	int err;

	err = i2c_smbus_read_byte_data(client, SI1143_PART_ID);
	if (err < 0)
		return err;
	if (err != SI1143_PART_ID_VAL)
	{
		return -ENODEV;
	}

	// Send the 0x017 to HW_KEY to enter Standby mode
  	err = i2c_smbus_write_byte_data(client, SI1143_HW_KEY, 0x17);
	if (err < 0)
	{
		return err;
	}

	// Set the INT_CFG register
	// level, High-Z
	err = i2c_smbus_write_byte_data(client, SI1143_INT_CFG, 0x01);
	if (err < 0)
	{
		return err;
	}
  
  
	// Set the IRQ_ENABLE register
	err = i2c_smbus_write_byte_data(client, SI1143_IRQ_ENABLE, 0x00);
	if (err < 0)
	{
		return err;
	}
  
	// Set the IRQ_MODE1 register
	err = i2c_smbus_write_byte_data(client, SI1143_IRQ_MODE1, 0x00);
	if (err < 0)
	{
		return err;
	}
  
	// Set the PS_LED21 register (SET LED CURRENT)
	// LED1_I set to 100%
	err = i2c_smbus_write_byte_data(client, SI1143_PS_LED21, 
									(platform_data->proximity_tx_led1 ? 0x0F : 0) |
									(platform_data->proximity_tx_led2 ? 0xF0 : 0) );
	if (err < 0)
	{
		return err;
	}

	err = i2c_smbus_write_byte_data(client, SI1143_PS_LED3, 
									(platform_data->proximity_tx_led3 ? 0x0F : 0) );
	if (err < 0)
	{
		return err;
	}

	// set PS1 in CHLIST to enable LED1 measurement
  
	// first set the value of PARAM_WR reg
	// turn on ALS_IR, ALS_VIS and PS1 measurement
	err = i2c_smbus_write_byte_data(client, SI1143_PARAM_WR, 0x31);
	if (err < 0)
	{
		return err;
	}
  
	// next send PARAM_SET command
	// turn on PS1 measurement
	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PARAM_SET | SI1143_CHLIST);
	if (err < 0)
	{
		return err;
	}
  
	err = i2c_smbus_write_byte_data(client, SI1143_PARAM_WR, 
									(platform_data->proximity_tx_led1 ? 0x01 : 0) |
									(platform_data->proximity_tx_led2 ? 0x02 : 0) |
									(platform_data->proximity_tx_led3 ? 0x04 : 0) );
	if (err < 0)
	{
		return err;
	}

	// next send PARAM_SET command
	// turn on PS1 measurement
	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PARAM_SET | SI1143_PSLED12_SELECT);
	if (err < 0)
	{
		return err;
	}

	// first set the value of PARAM_WR reg
	err = i2c_smbus_write_byte_data(client, SI1143_PARAM_WR, SI1143_DEFAULT_PS_ADC_GAIN);
	if (err < 0)
	{
		return err;
	}

	// next send PARAM_SET command
	// set the PS_ADC_GAIN value
	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PARAM_SET | SI1143_PS_ADC_GAIN);
	if (err < 0)
	{
		return err;
	}

	// first set the value of PARAM_WR reg
	err = i2c_smbus_write_byte_data(client, SI1143_PARAM_WR, SI1143_DEFAULT_ALS_VS_ADC_GAIN);
	if (err < 0)
	{
		return err;
	}

	// next send PARAM_SET command
	// set the ALS_VIS_ADC_GAIN value
	err = i2c_smbus_write_byte_data(client, SI1143_COMMAND, SI1143_PARAM_SET | SI1143_ALS_VIS_ADC_GAIN);
	if (err < 0)
	{
		return err;
	}


  
	return 0;
}

static void si1143_i2c_reschedule_work(struct si1143_data *data,
					  unsigned long delay)
{
	unsigned long flags;

	spin_lock_irqsave(&data->lock, flags);

	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&data->dwork);
	schedule_delayed_work(&data->dwork, delay);

	spin_unlock_irqrestore(&data->lock, flags);
}

static irqreturn_t si1143_irq(int irq, void *dev_id)
{
	struct si1143_data *data = dev_id;

	si1143_i2c_reschedule_work(data, 0);

	return IRQ_HANDLED;
}

static void si1143_i2c_work_handler(struct work_struct *work)
{
	struct si1143_data *data = container_of(work, struct si1143_data, dwork.work);
	struct i2c_client *client = data->client;
	int results;
	
	//Get the interrupts over i2c
	results = i2c_smbus_read_byte_data(client, SI1143_IRQ_STATUS);

	//Process interrupts
	//printk("si1143 irq handler 0x%X\n", results);

	//Clear the interrupts
	i2c_smbus_write_byte_data(client, SI1143_IRQ_STATUS, results);
}

/*
 * I2C init/probing/exit functions
 */

static struct i2c_driver si1143_driver;

static int __devinit si1143_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct si1143_data *data;
	int err = 0;
	struct si1143_platform_data *platform_data;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WRITE_BYTE
					    | I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
		err = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct si1143_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	data->client = client;
	i2c_set_clientdata(client, data);

	/* Check platform data */
	platform_data = client->dev.platform_data;

	mutex_init(&data->update_lock);

	data->als_interrupt_spectrum=0;

	data->irq = gpio_to_irq(client->irq);

	/* request the GPIOs */
	err = gpio_request(client->irq, SI1143_DRV_NAME);
	if (err) {
		dev_err(&client->dev, "unable to request GPIO %d\n",
			client->irq);
		goto exit_kfree;
	}

	/* request the IRQs */
	err = request_irq(data->irq, &si1143_irq,
			  IORESOURCE_IRQ_HIGHEDGE | IORESOURCE_IRQ_LOWEDGE,
			  SI1143_DRV_NAME, data);
	if (err) {
		dev_err(&client->dev, "unable to request IRQ %d\n",
			data->irq);
		gpio_free(client->irq);
		goto exit_kfree;
	}


	/* Initialize the SI1143 chip */
	err = si1143_init_client(client, platform_data);
	if (err)
	{
		goto exit_kfree;
	}

	INIT_DELAYED_WORK(&data->dwork, si1143_i2c_work_handler);
	spin_lock_init(&data->lock);

	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &si1143_attr_group);
	if (err)
	{
		goto exit_kfree;
	}

	dev_info(&client->dev, "support ver. %s enabled\n", DRIVER_VERSION);

	return 0;

exit_kfree:
	kfree(data);
exit:
	return err;
}

static int __devexit si1143_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &si1143_attr_group);

	/* Power down the device */
  //  si1143_set_power_state(client, 0);

	kfree(i2c_get_clientdata(client));

	return 0;
}

#ifdef CONFIG_PM

static int si1143_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct si1143_data *data = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&data->dwork);

	return 0;//si1143_set_power_state(client, 0);
}

static int si1143_resume(struct i2c_client *client)
{
	return 0;//si1143_set_power_state(client, 1);
}

#else

#define si1143_suspend		NULL
#define si1143_resume		NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id si1143_id[] = {
	{ "si1143", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, si1143_id);

static struct i2c_driver si1143_driver = {
	.driver = {
		.name	= SI1143_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.suspend = si1143_suspend,
	.resume	= si1143_resume,
	.probe	= si1143_probe,
	.remove	= __devexit_p(si1143_remove),
	.id_table = si1143_id,
};

static int __init si1143_init(void)
{
	return i2c_add_driver(&si1143_driver);
}

static void __exit si1143_exit(void)
{
	i2c_del_driver(&si1143_driver);
}

MODULE_AUTHOR("Nest Labs, Inc");
MODULE_DESCRIPTION("SI1143 ambient light and proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(si1143_init);
module_exit(si1143_exit);
