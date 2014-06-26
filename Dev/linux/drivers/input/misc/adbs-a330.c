/*
 *  adbs-a330.c - Linux kernel modules for Avago ADBS A330 sensor
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
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/adbs-a330.h>
#include <linux/regulator/consumer.h>

#define DRV_NAME "avago-adbs-a330"
#define ENCODER_REPORT_TIME (HZ/60)

enum {
	REG_PRODUCT_ID = 0x00,
	REG_REVISION_ID = 0x01,
	REG_MOTION = 0x02,
	REG_DELTA_X = 0x03,
	REG_DELTA_Y = 0x04,
	REG_SQUAL = 0x05,
	REG_SHUTTER_UPPER = 0x06,
	REG_SHUTTER_LOWER = 0x07,
	REG_MAXIMUM_PIXEL = 0x08,
	REG_PIXEL_SUM = 0x09,
	REG_MINIMUM_PIXEL = 0x0a,
	REG_PIXEL_GRAB = 0x0b,
	REG_CRC0 = 0x0c,
	REG_CRC1 = 0x0d,
	REG_CRC2 = 0x0e,
	REG_CRC3 = 0x0f,
	REG_SELF_TEST = 0x10,
	REG_CONFIGURATION_BITS = 0x11,
	REG_RESERVED0 = 0x12,
	REG_RESERVED1 = 0x13,
	REG_RESERVED2 = 0x14,
	REG_RESERVED3 = 0x15,
	REG_RESERVED4 = 0x16,
	REG_RESERVED5 = 0x17,
	REG_RESERVED6 = 0x18,
	REG_RESERVED7 = 0x19,
	REG_LED_CONTROL = 0x1a,
	REG_RESERVED8 = 0x1b,
	REG_IO_MODE = 0x1c,
	REG_MOTION_CONTROL = 0x1d,
	REG_RESERVED9 = 0x1e,
	REG_RESERVED10 = 0x1f,
	REG_RESERVED11 = 0x20,
	REG_RESERVED12 = 0x21,
	REG_RESERVED13 = 0x22,
	REG_RESERVED14 = 0x23,
	REG_RESERVED15 = 0x24,
	REG_RESERVED16 = 0x25,
	REG_RESERVED17 = 0x26,
	REG_RESERVED18 = 0x27,
	REG_RESERVED19 = 0x28,
	REG_RESERVED20 = 0x29,
	REG_RESERVED21 = 0x2a,
	REG_RESERVED22 = 0x2b,
	REG_RESERVED23 = 0x2c,
	REG_RESERVED24 = 0x2d,
	REG_OBSERVATION = 0x2e,
	REG_RESERVED25 = 0x2f,
	REG_RESERVED26 = 0x30,
	REG_RESERVED27 = 0x31,
	REG_RESERVED28 = 0x32,
	REG_RESERVED29 = 0x33,
	REG_RESERVED30 = 0x34,
	REG_RESERVED31 = 0x35,
	REG_RESERVED32 = 0x36,
	REG_RESERVED33 = 0x37,
	REG_RESERVED34 = 0x38,
	REG_RESERVED35 = 0x39,
	REG_SOFT_RESET = 0x3a,
	REG_SHUTTER_MAX_HI = 0x3b,
	REG_SHUTTER_MAX_LO = 0x3c,
	REG_RESERVED36 = 0x3d,
	REG_INVERSE_REVISION_ID = 0x3e,
	REG_INVERSE_PRODUCT_ID = 0x3f,
	REG_RESERVED37 = 0x40,
	REG_RESERVED38 = 0x41,
	REG_RESERVED39 = 0x42,
	REG_RESERVED40 = 0x43,
	REG_RESERVED41 = 0x44,
	REG_RESERVED42 = 0x45,
	REG_RESERVED43 = 0x46,
	REG_RESERVED44 = 0x47,
	REG_RESERVED45 = 0x48,
	REG_RESERVED46 = 0x49,
	REG_RESERVED47 = 0x4a,
	REG_RESERVED48 = 0x4b,
	REG_RESERVED49 = 0x4c,
	REG_RESERVED50 = 0x4d,
	REG_RESERVED51 = 0x4e,
	REG_RESERVED52 = 0x4f,
	REG_RESERVED53 = 0x50,
	REG_RESERVED54 = 0x51,
	REG_RESERVED55 = 0x52,
	REG_RESERVED56 = 0x53,
	REG_RESERVED57 = 0x54,
	REG_RESERVED58 = 0x55,
	REG_RESERVED59 = 0x56,
	REG_RESERVED60 = 0x57,
	REG_RESERVED61 = 0x58,
	REG_RESERVED62 = 0x59,
	REG_RESERVED63 = 0x5a,
	REG_RESERVED64 = 0x5b,
	REG_RESERVED65 = 0x5c,
	REG_RESERVED66 = 0x5d,
	REG_RESERVED67 = 0x5e,
	REG_RESERVED68 = 0x5f,
	REG_OFN_ENGINE = 0x60,
	REG_OFN_ENGINE2 = 0x61,
	REG_OFN_RESOLUTION = 0x62,
	REG_OFN_SPEED_CONTROL = 0x63,
	REG_OFN_SPEED_ST12 = 0x64,
	REG_OFN_SPEED_ST21 = 0x65,
	REG_OFN_SPEED_ST23 = 0x66,
	REG_OFN_SPEED_ST32 = 0x67,
	REG_OFN_SPEED_ST34 = 0x68,
	REG_OFN_SPEED_ST43 = 0x69,
	REG_OFN_SPEED_ST45 = 0x6a,
	REG_OFN_SPEED_ST54 = 0x6b,
	REG_GPIO_CTRL = 0x6c,
	REG_AD_CTRL = 0x6d,
	REG_OFN_AD_ATH_HIGH = 0x6e,
	REG_OFN_AD_DTH_HIGH = 0x6f,
	REG_OFN_AD_ATH_LOW = 0x70,
	REG_OFN_AD_DTH_LOW = 0x71,
	REG_RESERVED71 = 0x72,
	REG_OFN_QUANTIZE_CTRL = 0x73,
	REG_OFN_XYQ_THRESHOLD = 0x74,
	REG_OFN_FPD_CTRL = 0x75,
	REG_RESERVED72 = 0x76,
	REG_OFN_ORIENTATION_CONTROL = 0x77,	
};

#define ADBS_A320_PRODUCT_ID (0x83)
#define ADBS_A320_REVISION_ID (0x01)
#define ADBS_A350_PRODUCT_ID (0x88)
#define ADBS_A350_REVISION_ID (0x00)

#define ADBS_A330_MOTION_MOT (0x1<<7)

#define ADBS_A330_VDDA_UV_MIN	2600000
#define ADBS_A330_VDDA_UV_MAX	3300000

struct adbs_a330_data {
	struct input_dev *input;
	struct i2c_client *client;
	struct adbs_a330_platform_data *pdata;
	struct delayed_work	dwork;
	spinlock_t		lock;
	struct timer_list timer;
	struct regulator* vdd;
	uint32_t irq_a;
	int32_t accumulatedPosition;
	int32_t lastPositionSent;
	int direction;
	int mode;
	int reset_gpio;
	int shutdown_gpio;
	int motion_gpio;
};

// forward declarations
static ssize_t adbs_a330_get_direction(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t adbs_a330_set_direction(struct device *dev, struct device_attribute *attr, const char *buf, size_t len);
static ssize_t adbs_a330_get_mode(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t adbs_a330_set_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t len);

static DEVICE_ATTR(direction, (S_IRUGO|S_IWUGO), adbs_a330_get_direction, adbs_a330_set_direction);
static DEVICE_ATTR(mode, (S_IRUGO|S_IWUGO), adbs_a330_get_mode, adbs_a330_set_mode);

static struct attribute *adbs_a330_attributes[] = {
	&dev_attr_direction.attr,
	&dev_attr_mode.attr,
	NULL
};

static const struct attribute_group adbs_a330_attr_group = {
	.attrs = adbs_a330_attributes,
};

static ssize_t adbs_a330_get_direction(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	const struct adbs_a330_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->direction);
}

static ssize_t adbs_a330_set_direction(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	u32 new_direction;
	struct i2c_client *client = to_i2c_client(dev);
	struct adbs_a330_data *data = i2c_get_clientdata(client);

	sscanf(buf, "%d", &new_direction);
	data->direction = new_direction;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t adbs_a330_get_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	const struct adbs_a330_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->mode);
}

static ssize_t adbs_a330_set_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	u32 new_mode;
	struct i2c_client *client = to_i2c_client(dev);
	struct adbs_a330_data *data = i2c_get_clientdata(client);

	sscanf(buf, "%u", &new_mode);
	data->mode = new_mode;
	return strnlen(buf, PAGE_SIZE);
}

static void adbs_a330_i2c_reschedule_work(struct adbs_a330_data *data,
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

static irqreturn_t adbs_a330_irq(int irq, void *dev_id)
{
	struct adbs_a330_data *data = dev_id;

	adbs_a330_i2c_reschedule_work(data, 0);

	return IRQ_HANDLED;
}


static void adbs_a330_timer(unsigned long private)
{
	struct adbs_a330_data* rot = (void*) private;
	int32_t newPosition = rot->accumulatedPosition;
	int32_t delta;

	if( newPosition != rot->lastPositionSent )
	{
		delta = (newPosition - rot->lastPositionSent);
		delta = (rot->direction == ADBS_A330_DIRECTION_NEG) ? -delta : delta;

		input_report_rel(rot->input, REL_X, delta);
		input_sync(rot->input);

		rot->lastPositionSent = newPosition;
	}

	mod_timer( &rot->timer, jiffies + ENCODER_REPORT_TIME );
}

static void adbs_a330_i2c_work_handler(struct work_struct *work)
{
	struct adbs_a330_data* rot = container_of(work, struct adbs_a330_data, dwork.work);
	struct i2c_client *client = rot->client;
	int val;
	int8_t val_x, val_y;
	int32_t delta_x = 0, delta_y = 0;
	do 
	{
		val = i2c_smbus_read_byte_data(client,  REG_DELTA_X);
		val_x = (int8_t) ((val << 24) >> 24);
		delta_x += (int32_t) val_x;
		if( val < 0 )
			return;

		val = i2c_smbus_read_byte_data(client,  REG_DELTA_Y);
		val_y = (int8_t) ((val << 24) >> 24);
		delta_y += (int32_t) val_y;
		if( val < 0 )
			return;

		if( ( val_x == 0 )
			&& ( val_y == 0 ) )
		{
			break;
		}

	} while( 1 );

	if( rot->mode == ADBS_A330_DELTA_X )
		rot->accumulatedPosition += delta_x;
	else
		rot->accumulatedPosition += delta_y;
}

static int __devinit adbs_a330_gpio_input_request_and_export(unsigned gpio, const char *name, struct device *dev, const char *link)
{
	int status = 0;
	const bool direction_may_change = true;

	status = gpio_request(gpio, name);
	if (status) {
		dev_err(dev, "Could not request GPIO %u: %d\n", gpio, status);
		goto done;
	}

	status = gpio_direction_input(gpio);
	if (status) {
		dev_err(dev, "Could not set GPIO %u as input: %d\n", gpio, status);
		goto free_gpio;
	}

	status = gpio_export(gpio, !direction_may_change);
	if (status) {
		dev_err(dev, "Could not export GPIO %u: %d\n", gpio, status);
		goto free_gpio;
	}

	status = gpio_export_link(dev, link, gpio);
	if (status) {
		dev_err(dev, "Could not export GPIO %u as link %s: %d\n", gpio, link, status);
		goto unexport_gpio;
	}

	goto done;

 unexport_gpio:
	gpio_unexport(gpio);

 free_gpio:
	gpio_free(gpio);

 done:
	return status;
}

static int __devinit adbs_a330_gpio_output_request_and_export(unsigned gpio, const char *name, int value, struct device *dev, const char *link)
{
	int status = 0;
	const bool direction_may_change = true;

	status = gpio_request(gpio, name);
	if (status) {
		dev_err(dev, "Could not request GPIO %u: %d\n", gpio, status);
		goto done;
	}

	status = gpio_direction_output(gpio, value);
	if (status) {
		dev_err(dev, "Could not set GPIO %u as output to value %d: %d\n", gpio, value, status);
		goto free_gpio;
	}

	status = gpio_export(gpio, !direction_may_change);
	if (status) {
		dev_err(dev, "Could not export GPIO %u: %d\n", gpio, status);
		goto free_gpio;
	}

	status = gpio_export_link(dev, link, gpio);
	if (status) {
		dev_err(dev, "Could not export GPIO %u as link %s: %d\n", gpio, link, status);
		goto unexport_gpio;
	}

	goto done;

 unexport_gpio:
	gpio_unexport(gpio);

 free_gpio:
	gpio_free(gpio);

 done:
	return status;
}

static void adbs_a330_gpio_unexport_and_free(unsigned gpio)
{
	gpio_unexport(gpio);
	gpio_free(gpio);
}

static int __devinit adbs_a350_init(struct i2c_client *client)
{
	int val = 0;

	//Write 0xe4 to 0x60 and 0xC9 to 0x61 as part of start up sequence
	val = 0xe4;
	i2c_smbus_write_byte_data(client, REG_OFN_ENGINE, val);

	udelay(30);

	val = 0xc9;
	i2c_smbus_write_byte_data(client, REG_OFN_ENGINE2, val);

	udelay(30);

	//Write 0x00 to REG_GPIO_CTRL to set GPIO pin as active low output
	val = 0x00;
	i2c_smbus_write_byte_data(client, REG_GPIO_CTRL, val);

	udelay(30);

	//Write 0x0e to REG_MOTION_CONTROL to turn off soft click int and button click int
	val = 0x0e;
	i2c_smbus_write_byte_data(client, REG_MOTION_CONTROL, val);

	udelay(30);

	//Set OFN Resolution
	val = i2c_smbus_read_byte_data(client, 	REG_OFN_RESOLUTION);
	if( (int) val < 0 )
		return -ENODEV;
	val &= ~(0xF<<4);
	val |= (0x1<<4);
	i2c_smbus_write_byte_data(client, REG_OFN_RESOLUTION, val);

	udelay(30);

        //Turn on manual resolution control
	val = 0x84;
	i2c_smbus_write_byte_data(client, REG_OFN_ENGINE, val);

	return 0;
}

static int __devinit adbs_a320_init(struct i2c_client *client)
{
	int val = 0;
	//Set OFN Resolution
	val = i2c_smbus_read_byte_data(client, 	REG_OFN_RESOLUTION);
	if( (int) val < 0 )
		return -ENODEV;
	val &= ~(0x7<<3);
	val |= (0x1<<3);
	i2c_smbus_write_byte_data(client, REG_OFN_RESOLUTION, val);

	return 0;
}
static int __devinit adbs_a330_init_regulator(struct i2c_client *client, const char *supply, int uv)
{
	const int min_uv = ADBS_A330_VDDA_UV_MIN;
	const int max_uv = ADBS_A330_VDDA_UV_MAX;
	int err = 0;
	struct adbs_a330_data *rot = i2c_get_clientdata(client);
	struct regulator *reg;
	int actual_uv;

	/* Validate that the requested supply voltage is within the
	 * allowed range of the part. We can do this without checking it
	 * against the regulator.
	 */

	if (uv < min_uv || uv > max_uv) {
		dev_err(&client->dev, "The specified voltage %duV is out of the "
				"allowed range [%duV, %duV] for this device.\n", uv, min_uv,
				max_uv);
		err = -EINVAL;
		goto exit_done;
	}

	/* Attempt to get a reference to the regulator associated with the
	 * specified supply.
	 */

	reg = regulator_get(&client->dev, supply);

	if (IS_ERR(reg)) {
		err = PTR_ERR(reg);
		dev_err(&client->dev, "Could not get requested regulator supply '%s': "
				"%d\n", supply, err);
		goto exit_done;
	}

	/* Validate that regulator supports the range of voltages required
	 * by the device.
	 */

	err = regulator_is_supported_voltage(reg, min_uv, max_uv);
	if (err <= 0) {
		dev_err(&client->dev, "The regulator associated with the voltage "
				"supply '%s' cannot support the required voltage range [%d, "
				"%d] for this device.\n", supply, min_uv, max_uv);
		err = ((err == 0) ? -EINVAL : err);
		goto exit_free_regulator;
	}

	/* Finally, request the specified voltage and report whether we got
	 * exactly that or something else.
	 */

	err = regulator_set_voltage(reg, uv, uv);
	if (err) {
		dev_err(&client->dev, "The regulator associated with the voltage "
				"supply '%s' could not be set to the requested voltage %d: "
				"%d\n", supply, uv, err);
		goto exit_free_regulator;
	}

	actual_uv = regulator_get_voltage(reg);

	if (actual_uv != uv) {
		dev_warn(&client->dev, "Requested %duV from regulator supply '%s' but "
				 "actually got %duV.\n", uv, supply, actual_uv);
	}

	dev_info(&client->dev, "Successfully set regulator supply '%s' to %duV.\n",
			 supply, actual_uv);

	rot->vdd = reg;

	goto exit_done;

exit_free_regulator:
	regulator_put(reg);

exit_done:
	return err;
}

static int __devinit adbs_a330_init_client(struct i2c_client *client)
{
	int err = 0;
	int product_id = 0;
	unsigned int val;
	int retval = 0;
	const struct adbs_a330_data *rot = i2c_get_clientdata(client);

	if (rot->vdd != NULL)
	{
		err = regulator_enable(rot->vdd);
		if (err) {
			dev_err(&client->dev, "Could not enable the voltage supply regulator: %d\n", err);
				return err;
		}
	}

	/* XXX
	   random 20usec delay to let the allow settling of the power rails
	   probably not needed.*/
	udelay(20);
	/* power up the sensor */
	gpio_set_value(rot->shutdown_gpio, 0);
	/* worst case 500usec according to datasheet. Double that for good measure. */
	udelay(1000);

	/* pulse the reset gpio */
	gpio_set_value(rot->reset_gpio, 0);
	udelay(20);
	gpio_set_value(rot->reset_gpio, 1);

	product_id = i2c_smbus_read_byte_data(client, REG_PRODUCT_ID);
	if (product_id < 0)
		return product_id;
        
	if ( ( product_id != ADBS_A320_PRODUCT_ID )
			&& (product_id != ADBS_A350_PRODUCT_ID) )
	{
		return -ENODEV;
	}
	
	err = i2c_smbus_read_byte_data(client, REG_REVISION_ID);
	if (err < 0)
		return err;
	if ( (err != ADBS_A320_REVISION_ID) && (err != ADBS_A350_REVISION_ID) )
	{
		return -ENODEV;
	}

	udelay(500);

	val = i2c_smbus_read_byte_data(client, 	REG_OFN_SPEED_CONTROL);
	
	if (product_id == ADBS_A350_PRODUCT_ID)
	{
	    if((retval = adbs_a350_init(client))) return retval;
	}

	if (product_id == ADBS_A320_PRODUCT_ID)
	{
	    if((retval = adbs_a320_init(client))) return retval;
	}

	udelay(20);

	val = i2c_smbus_read_byte_data(client, 	REG_OFN_SPEED_CONTROL);
	if( (int) val < 0 )
		return -ENODEV;
	val |= (0x1<<1);
	val &= ~(0x1<<0);
	i2c_smbus_write_byte_data(client, REG_OFN_SPEED_CONTROL, val);
	return 0;
}


static int __devinit adbs_a330_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct input_dev *input;
	struct adbs_a330_data *data;
	int err = 0;
	struct adbs_a330_platform_data *platform_data;
	unsigned gpio;
	int value;
	const char *name, *link;

	/* Check platform data */
	platform_data = client->dev.platform_data;

	if (platform_data == NULL) {
		dev_err(&client->dev, "Cannot obtain platform data!\n");
		err = -ENODEV;
		goto exit_end;
	}

	printk("Avago ADBS driver\n");
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WRITE_BYTE
					    | I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
		err = -EIO;
		goto exit_end;
	}

	data = kzalloc(sizeof(struct adbs_a330_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "failed to allocate memory for device\n");
		err = -ENOMEM;
		goto exit_end;
	}
	data->client = client;
	i2c_set_clientdata(client, data);

	if (platform_data->vdda_supply != NULL)
	{
		/* make sure we request the regulator for the analog supply rail
		 * we are using
		 */
		err = adbs_a330_init_regulator(client,
								       platform_data->vdda_supply,
								       platform_data->vdda_uv);
		if (err) {
			dev_err(&client->dev, "Failed to initialize analog voltage supply: %d\n", err);
			goto exit_free_mem;
		}
	}

	input = input_allocate_device();
	if (!input) {
		dev_err(&client->dev, "failed to allocate memory for device\n");
		err = -ENOMEM;
		goto exit_free_regulator;
	}

	// Request and export the reset GPIO

	gpio = platform_data->reset_gpio;
	value = 1;
	name = "ADBS A330 Reset#";
	link = "reset#";

	err = adbs_a330_gpio_output_request_and_export(gpio, name, value, &client->dev, link);
	if (err) {
		dev_err(&client->dev, "Unable to request and export GPIO %d for output: %d\n", gpio, err);
		goto exit_free_input;
	}

	data->reset_gpio = gpio;

	// Request and export the shutdown GPIO

	gpio = platform_data->shutdown_gpio;
	value = 1;
	name = "ADBS A330 Shutdown";
	link = "shutdown";

	err = adbs_a330_gpio_output_request_and_export(gpio, name, value, &client->dev, link);
	if (err) {
		dev_err(&client->dev, "Unable to request and export GPIO %d for output: %d\n", gpio, err);
		goto exit_free_reset_gpio;
	}

	data->shutdown_gpio = gpio;

	// Request and export the motion GPIO

	gpio = platform_data->motion_gpio;
	name = "ADBS A330 Motion#";
	link = "motion#";

	err = adbs_a330_gpio_input_request_and_export(gpio, name, &client->dev, link);
	if (err) {
		dev_err(&client->dev, "Unable to request and export GPIO %d for input: %d\n", gpio, err);
		goto exit_free_shutdown_gpio;
	}

	err = enable_irq_wake(gpio_to_irq(gpio));
	if (err) {
		dev_err(&client->dev, "Failed to :enable_irq_wake\n");
		goto exit_free_motion_gpio;
	}

	data->motion_gpio = gpio;

	/* create and register the input driver */
	data->input = input;
	input->name = client->name;
	input->id.bustype = BUS_HOST;
	input->dev.parent = &client->dev;

    input->evbit[0] = BIT_MASK(EV_REL);
    input->relbit[0] = BIT_MASK(0 /* axis */);

	err = input_register_device(input);
	if (err) {
		dev_err(&client->dev, "failed to register input device\n");
		goto exit_disable_wake;
	}

	/* Initialize the Avago chip */
	err = adbs_a330_init_client(client);
	if (err) {
		dev_err(&client->dev, "Could not initialize I2C client: %d\n", err);
		goto exit_unregister_input;
	}
        
	data->accumulatedPosition = 0;
	data->lastPositionSent = 0;
	data->direction = platform_data->direction;
	data->mode = platform_data->mode;

	INIT_DELAYED_WORK(&data->dwork, adbs_a330_i2c_work_handler);
	spin_lock_init(&data->lock);

	/* request the IRQs */
	data->irq_a = gpio_to_irq(platform_data->motion_gpio);

	err = request_irq(data->irq_a, &adbs_a330_irq,
					  IORESOURCE_IRQ_LOWEDGE,
					  DRV_NAME, data);
	if (err) {
		dev_err(&client->dev, "unable to request IRQ %d\n", data->irq_a);
		goto exit_unregister_input;
	}

	init_timer(&data->timer);
	data->timer.data = (long) data;
	data->timer.function = adbs_a330_timer;
	mod_timer( &data->timer, jiffies + ENCODER_REPORT_TIME );

	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &adbs_a330_attr_group);
	if (err) {
		goto exit_free_irq;
	}

	return 0;

exit_free_irq:
	del_timer_sync(&data->timer);
	free_irq(data->irq_a, data);

exit_unregister_input:
	input_unregister_device(input);
	input = NULL; /* so we don't try to free it */
exit_disable_wake:
    disable_irq_wake(gpio_to_irq(data->motion_gpio));	
exit_free_motion_gpio:
	adbs_a330_gpio_unexport_and_free(platform_data->motion_gpio);

exit_free_shutdown_gpio:
	adbs_a330_gpio_unexport_and_free(platform_data->shutdown_gpio);

exit_free_reset_gpio:
	adbs_a330_gpio_unexport_and_free(platform_data->reset_gpio);

exit_free_input:
	input_free_device(input);

exit_free_regulator:
	if (data->vdd != NULL)
		regulator_put(data->vdd);

exit_free_mem:
	kfree(data);

exit_end:
	return err;
}

static int __devexit adbs_a330_remove(struct i2c_client *client)
{
	struct adbs_a330_data *rot = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &adbs_a330_attr_group);
	del_timer_sync(&rot->timer);
	free_irq(rot->irq_a, rot);
	adbs_a330_gpio_unexport_and_free(rot->motion_gpio);
	adbs_a330_gpio_unexport_and_free(rot->shutdown_gpio);
	adbs_a330_gpio_unexport_and_free(rot->reset_gpio);
	input_unregister_device(rot->input);

	if (rot->vdd != NULL)
		regulator_put(rot->vdd);
		  
	platform_set_drvdata(client, NULL);
	kfree(rot);

	return 0;
}

static const struct i2c_device_id adbs_a330_id[] = {
	{ "avago-adbs-a330", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, adbs_a330_id);

static struct i2c_driver adbs_a330_driver = {
	.driver = {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= adbs_a330_probe,
	.remove	= __devexit_p(adbs_a330_remove),
	.id_table = adbs_a330_id,
};

static int __init adbs_a330_init(void)
{
	return i2c_add_driver(&adbs_a330_driver);
}

static void __exit adbs_a330_exit(void)
{
	i2c_del_driver(&adbs_a330_driver);
}

module_init(adbs_a330_init);
module_exit(adbs_a330_exit);


MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DESCRIPTION("Driver for Avago ADBS-A330 Avago motion sensor");
MODULE_AUTHOR("Andrea Mucignat <andrea@nestlabs.com>");
MODULE_LICENSE("GPLv2");

