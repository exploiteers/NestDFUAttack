/*
 * rotary_encoder_lite.c
 *
 *  XXX add copyright
 *
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/rotary_encoder_lite.h>

#define DRV_NAME "rotary-encoder-lite"
#define ENCODER_REPORT_TIME (HZ/60)

struct rotary_encoder_lite {
	struct input_dev *input;
	struct rotary_encoder_lite_platform_data *pdata;
	struct timer_list timer;
	uint32_t irq_a;
	uint8_t accumulatedPosition;
	uint8_t lastPositionSent;
};

static irqreturn_t rotary_encoder_lite_irq(int irq, void *dev_id)
{
	struct rotary_encoder_lite *encoder = dev_id;
	struct rotary_encoder_lite_platform_data *pdata = encoder->pdata;
	int direction = !!gpio_get_value(pdata->gpio_b);
	encoder->accumulatedPosition += (direction ? -1 : 1 );
	
	// clear the flip-flop
	gpio_set_value(encoder->pdata->gpio_clear, 0);
	gpio_set_value(encoder->pdata->gpio_clear, 1);

	return IRQ_HANDLED;
}

static void encoder_timer(unsigned long private)
{
	struct rotary_encoder_lite* enc = (void*) private;

	uint8_t newPosition = enc->accumulatedPosition;
	if( newPosition != enc->lastPositionSent )
	{
		input_report_rel( enc->input,
						  0, // axis
						  (int8_t)(newPosition - enc->lastPositionSent));
		input_sync( enc->input );
		enc->lastPositionSent = newPosition;
	}
	mod_timer( &enc->timer, jiffies + ENCODER_REPORT_TIME );
}

static int __devinit rotary_encoder_lite_probe(struct platform_device *pdev)
{
	struct rotary_encoder_lite_platform_data *pdata = pdev->dev.platform_data;
	struct rotary_encoder_lite *encoder;
	struct input_dev *input;
	int err;

	if (!pdata) {
		dev_err(&pdev->dev, "missing platform data\n");
		return -ENOENT;
	}

	encoder = kzalloc(sizeof(struct rotary_encoder_lite), GFP_KERNEL);
	input = input_allocate_device();
	if (!encoder || !input) {
		dev_err(&pdev->dev, "failed to allocate memory for device\n");
		err = -ENOMEM;
		goto exit_free_mem;
	}

	encoder->input = input;
	encoder->pdata = pdata;
	encoder->irq_a = gpio_to_irq(pdata->gpio_a);

	/* create and register the input driver */
	input->name = pdev->name;
	input->id.bustype = BUS_HOST;
	input->dev.parent = &pdev->dev;

    input->evbit[0] = BIT_MASK(EV_REL);
    input->relbit[0] = BIT_MASK(0 /* axis */);

	err = input_register_device(input);
	if (err) {
		dev_err(&pdev->dev, "failed to register input device\n");
		goto exit_free_mem;
	}

	/* request the GPIOs */
	err = gpio_request(pdata->gpio_a, DRV_NAME);
	if (err) {
		dev_err(&pdev->dev, "unable to request GPIO %d\n",
			pdata->gpio_a);
		goto exit_unregister_input;
	}

	err = gpio_request(pdata->gpio_b, DRV_NAME);
	if (err) {
		dev_err(&pdev->dev, "unable to request GPIO %d\n",
			pdata->gpio_b);
		goto exit_free_gpio_a;
	}

    err = gpio_request(pdata->gpio_clear, DRV_NAME);
	if (err) {
		dev_err(&pdev->dev, "unable to request GPIO %d\n",
			pdata->gpio_b);
		goto exit_free_gpio_a;
	}
    gpio_set_value(pdata->gpio_clear, 1);

	/* request the IRQs */
	err = request_irq(encoder->irq_a, &rotary_encoder_lite_irq,
			  IORESOURCE_IRQ_HIGHEDGE,
			  DRV_NAME, encoder);
	if (err) {
		dev_err(&pdev->dev, "unable to request IRQ %d\n",
			encoder->irq_a);
		goto exit_free_gpio_b;
	}

	encoder->accumulatedPosition = 0;
	encoder->lastPositionSent = 0;

	init_timer(&encoder->timer);
	encoder->timer.data = (long) encoder;
	encoder->timer.function = encoder_timer;
	mod_timer( &encoder->timer, jiffies + ENCODER_REPORT_TIME );
	
	platform_set_drvdata(pdev, encoder);

	return 0;

exit_free_gpio_b:
	gpio_free(pdata->gpio_b);
exit_free_gpio_a:
	gpio_free(pdata->gpio_a);
exit_unregister_input:
	input_unregister_device(input);
	input = NULL; /* so we don't try to free it */
exit_free_mem:
	input_free_device(input);
	kfree(encoder);
	return err;
}

static int __devexit rotary_encoder_lite_remove(struct platform_device *pdev)
{
	struct rotary_encoder_lite *encoder = platform_get_drvdata(pdev);
	struct rotary_encoder_lite_platform_data *pdata = pdev->dev.platform_data;

	del_timer_sync(&encoder->timer);
	free_irq(encoder->irq_a, encoder);
	gpio_free(pdata->gpio_a);
	gpio_free(pdata->gpio_b);
	input_unregister_device(encoder->input);
	platform_set_drvdata(pdev, NULL);
	kfree(encoder);

	return 0;
}

static struct platform_driver rotary_encoder_lite_driver = {
	.probe		= rotary_encoder_lite_probe,
	.remove		= __devexit_p(rotary_encoder_lite_remove),
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	}
};

static int __init rotary_encoder_lite_init(void)
{
	return platform_driver_register(&rotary_encoder_lite_driver);
}

static void __exit rotary_encoder_lite_exit(void)
{
	platform_driver_unregister(&rotary_encoder_lite_driver);
}

module_init(rotary_encoder_lite_init);
module_exit(rotary_encoder_lite_exit);

MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DESCRIPTION("GPIO rotary encoder driver lite");
MODULE_AUTHOR("Andrea Mucignat <andrea@nestlabs.com>");
MODULE_LICENSE("GPL v2");

