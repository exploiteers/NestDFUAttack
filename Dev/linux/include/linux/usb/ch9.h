/*
 * This file holds USB constants and structures that are needed for
 * USB device APIs.  These are used by the USB device model, which is
 * defined in chapter 9 of the USB 2.0 specification and in the
 * Wireless USB 1.0 (spread around).  Linux has several APIs in C that
 * need these:
 *
 * - the master/host side Linux-USB kernel driver API;
 * - the "usbfs" user space API; and
 * - the Linux "gadget" slave/device/peripheral side driver API.
 *
 * USB 2.0 adds an additional "On The Go" (OTG) mode, which lets systems
 * act either as a USB master/host or as a USB slave/device.  That means
 * the master and slave side APIs benefit from working well together.
 *
 * There's also "Wireless USB", using low power short range radios for
 * peripheral interconnection but otherwise building on the USB framework.
 *
 * Note all descriptors are declared '__attribute__((packed))' so that:
 *
 * [a] they never get padded, either internally (USB spec writers
 *     probably handled that) or externally;
 *
 * [b] so that accessing bigger-than-a-bytes fields will never
 *     generate bus errors on any platform, even when the location of
 *     its descriptor inside a bundle isn't "naturally aligned", and
 *
 * [c] for consistency, removing all doubt even when it appears to
 *     someone that the two other points are non-issues for that
 *     particular descriptor type.
 */

#ifndef __LINUX_USB_CH9_H
#define __LINUX_USB_CH9_H

#include <linux/types.h>	/* __u8 etc */

/*-------------------------------------------------------------------------*/

/* CONTROL REQUEST SUPPORT */

/*
 * USB directions
 *
 * This bit flag is used in endpoint descriptors' bEndpointAddress field.
 * It's also one of three fields in control requests bRequestType.
 */
#define USB_DIR_OUT			0		/* to device */
#define USB_DIR_IN			0x80		/* to host */

/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03
/* From Wireless USB 1.0 */
#define USB_RECIP_PORT			0x04
#define USB_RECIP_RPIPE		0x05

/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

#define USB_REQ_SET_ENCRYPTION		0x0D	/* Wireless USB */
#define USB_REQ_GET_ENCRYPTION		0x0E
#define USB_REQ_RPIPE_ABORT		0x0E
#define USB_REQ_SET_HANDSHAKE		0x0F
#define USB_REQ_RPIPE_RESET		0x0F
#define USB_REQ_GET_HANDSHAKE		0x10
#define USB_REQ_SET_CONNECTION		0x11
#define USB_REQ_SET_SECURITY_DATA	0x12
#define USB_REQ_GET_SECURITY_DATA	0x13
#define USB_REQ_SET_WUSB_DATA		0x14
#define USB_REQ_LOOPBACK_DATA_WRITE	0x15
#define USB_REQ_LOOPBACK_DATA_READ	0x16
#define USB_REQ_SET_INTERFACE_DS	0x17

/* The Link Power Management (LPM) ECN defines USB_REQ_TEST_AND_SET command,
 * used by hubs to put ports into a new L1 suspend state, except that it
 * forgot to define its number ...
 */

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)  Hubs may also support a
 * new USB_REQ_TEST_AND_SET_FEATURE to put ports into L1 suspend.
 */
#define USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define USB_DEVICE_BATTERY		2	/* (wireless) */
#define USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE		3	/* (wireless)*/
#define USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

/*
 * New Feature Selectors as added by USB 3.0
 * See USB 3.0 spec Table 9-6
 */
#define USB_DEVICE_U1_ENABLE	48	/* dev may initiate U1 transition */
#define USB_DEVICE_U2_ENABLE	49	/* dev may initiate U2 transition */
#define USB_DEVICE_LTM_ENABLE	50	/* dev may send LTM */
#define USB_INTRF_FUNC_SUSPEND	0	/* function suspend */

#define USB_INTR_FUNC_SUSPEND_OPT_MASK	0xFF00

#define USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */

/* Bit array elements as returned by the USB_REQ_GET_STATUS request. */
#define USB_DEV_STAT_U1_ENABLED		2	/* transition into U1 state */
#define USB_DEV_STAT_U2_ENABLED		3	/* transition into U2 state */
#define USB_DEV_STAT_LTM_ENABLED	4	/* Latency tolerance messages */

/**
 * struct usb_ctrlrequest - SETUP data for a USB device control request
 * @bRequestType: matches the USB bmRequestType field
 * @bRequest: matches the USB bRequest field
 * @wValue: matches the USB wValue field (le16 byte order)
 * @wIndex: matches the USB wIndex field (le16 byte order)
 * @wLength: matches the USB wLength field (le16 byte order)
 *
 * This structure is used to send control requests to a USB device.  It matches
 * the different fields of the USB 2.0 Spec section 9.3, table 9-2.  See the
 * USB spec for a fuller description of the different fields, and what they are
 * used for.
 *
 * Note that the driver for any interface can issue control requests.
 * For most devices, interfaces don't coordinate with each other, so
 * such requests may be made at any time.
 */
struct usb_ctrlrequest {
	__u8 bRequestType;
	__u8 bRequest;
	__le16 wValue;
	__le16 wIndex;
	__le16 wLength;
} __attribute__ ((packed));

/*-------------------------------------------------------------------------*/

/*
 * STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, or
 * (rarely) accepted by SET_DESCRIPTOR.
 *
 * Note that all multi-byte values here are encoded in little endian
 * byte order "on the wire".  Within the kernel and when exposed
 * through the Linux-USB APIs, they are not converted to cpu byte
 * order; it is the responsibility of the client code to do this.
 * The single exception is when device and configuration descriptors (but
 * not other descriptors) are read from usbfs (i.e. /proc/bus/usb/BBB/DDD);
 * in this case the fields are converted to host endianness by the kernel.
 */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22
#define USB_DT_CS_RADIO_CONTROL		0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE		0x24
/* From the USB 3.0 spec */
#define	USB_DT_SS_ENDPOINT_COMP		0x30

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_DT_CS_DEVICE		(USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG		(USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING		(USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE		(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT		(USB_TYPE_CLASS | USB_DT_ENDPOINT)

/* All standard descriptors have these 2 fields at the beginning */
struct usb_descriptor_header {
	__u8  bLength;
	__u8  bDescriptorType;
} __attribute__ ((packed));

/*
 * USB Specification Release Number (bcdUSB)
 */
#define USB_BCD_REL10       	0x0100
#define USB_BCD_REL11       	0x0110
#define USB_BCD_REL20       	0x0200

/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE: Device descriptor */
struct usb_device_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__le16 bcdUSB;
	__u8  bDeviceClass;
	__u8  bDeviceSubClass;
	__u8  bDeviceProtocol;
	__u8  bMaxPacketSize0;
	__le16 idVendor;
	__le16 idProduct;
	__le16 bcdDevice;
	__u8  iManufacturer;
	__u8  iProduct;
	__u8  iSerialNumber;
	__u8  bNumConfigurations;
} __attribute__ ((packed));

#define USB_DT_DEVICE_SIZE		18


/*
 * Device and/or Interface Class codes
 * as found in bDeviceClass or bInterfaceClass
 * and defined by www.usb.org documents
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_MISC			0xef
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

#define USB_SUBCLASS_VENDOR_SPEC	0xff

/*-------------------------------------------------------------------------*/

/* USB_DT_CONFIG: Configuration descriptor information.
 *
 * USB_DT_OTHER_SPEED_CONFIG is the same descriptor, except that the
 * descriptor type is different.  Highspeed-capable devices can look
 * different depending on what speed they're currently running.  Only
 * devices with a USB_DT_DEVICE_QUALIFIER have any OTHER_SPEED_CONFIG
 * descriptors.
 */
struct usb_config_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__le16 wTotalLength;
	__u8  bNumInterfaces;
	__u8  bConfigurationValue;
	__u8  iConfiguration;
	__u8  bmAttributes;
	__u8  bMaxPower;
} __attribute__ ((packed));

#define USB_DT_CONFIG_SIZE		9

/* from config descriptor bmAttributes */
#define USB_CONFIG_ATT_ONE		(1 << 7)	/* must be set */
#define USB_CONFIG_ATT_SELFPOWER	(1 << 6)	/* self powered */
#define USB_CONFIG_ATT_WAKEUP		(1 << 5)	/* can wakeup */
#define USB_CONFIG_ATT_BATTERY		(1 << 4)	/* battery powered */

/*-------------------------------------------------------------------------*/

/*
 * USB Country Codes (bCountryCode)
 */
#define	USB_COUNTRY_NONE 			 0 /* None */
#define	USB_COUNTRY_ARABIC			 1 /* Arabic */
#define	USB_COUNTRY_BE				 2 /* Belgium */
#define	USB_COUNTRY_CA				 3 /* Canada */
#define	USB_COUNTRY_CAFR			 4 /* Canada/French */
#define	USB_COUNTRY_CZ				 5 /* Czech Republic */
#define	USB_COUNTRY_DK				 6 /* Danish */
#define	USB_COUNTRY_FI				 7 /* Finnish */
#define	USB_COUNTRY_FR				 8 /* France */
#define	USB_COUNTRY_DE				 9 /* Germany */
#define	USB_COUNTRY_GR				10 /* Greece */
#define	USB_COUNTRY_HEBREW			11 /* Hebrew */
#define	USB_COUNTRY_HU				12 /* Hungary */
#define	USB_COUNTRY_INTL			13 /* International (ISO) */
#define	USB_COUNTRY_IT				14 /* Italy */
#define	USB_COUNTRY_JP				15 /* Japan */
#define	USB_COUNTRY_KOREAN			16 /* Korea */
#define	USB_COUNTRY_LATIN			17 /* Latin American */
#define	USB_COUNTRY_NL				18 /* Netherlands */
#define	USB_COUNTRY_NO				19 /* Norway */
#define	USB_COUNTRY_PERSIAN			20 /* Persian (Farsi) */
#define	USB_COUNTRY_PL				21 /* Poland */
#define	USB_COUNTRY_PT				22 /* Portugal */
#define	USB_COUNTRY_RU				23 /* Russia */
#define	USB_COUNTRY_SK				24 /* Slovak Republic */
#define	USB_COUNTRY_ES				25 /* Spain */
#define	USB_COUNTRY_SE				26 /* Sweden */
#define	USB_COUNTRY_CHFR			27 /* Swiss/French */
#define	USB_COUNTRY_CHDE			28 /* Swiss/German */
#define	USB_COUNTRY_CH				29 /* Switzerland */
#define	USB_COUNTRY_TW				30 /* Taiwan */
#define	USB_COUNTRY_TRQ				31 /* Turkish-Q */
#define	USB_COUNTRY_UK				32 /* UK */
#define	USB_COUNTRY_US				33 /* US */
#define	USB_COUNTRY_YU				34 /* Yugoslavia */
#define	USB_COUNTRY_TRF				35 /* Turkish-F */

/*
 * USB Language Identifiers (wLANGID)
 */
#define	USB_LANG_AF					0x0436 /* Afrikaans */
#define	USB_LANG_SQ_AL				0x041c /* Albanian */
#define	USB_LANG_AR_SA				0x0401 /* Arabic (Saudi Arabia) */
#define	USB_LANG_AR_IQ				0x0801 /* Arabic (Iraq) */
#define	USB_LANG_AR_EG				0x0c01 /* Arabic (Egypt) */
#define	USB_LANG_AR_LY				0x1001 /* Arabic (Libya) */
#define	USB_LANG_AR_DZ				0x1401 /* Arabic (Algeria) */
#define	USB_LANG_AR_MA				0x1801 /* Arabic (Morocco) */
#define	USB_LANG_AR_TN				0x1c01 /* Arabic (Tunisia) */
#define	USB_LANG_AR_OM				0x2001 /* Arabic (Oman) */
#define	USB_LANG_AR_YE				0x2401 /* Arabic (Yemen) */
#define	USB_LANG_AR_SY				0x2801 /* Arabic (Syria) */
#define	USB_LANG_AR_JO				0x2c01 /* Arabic (Jordan) */
#define	USB_LANG_AR_LB				0x3001 /* Arabic (Lebanon) */
#define	USB_LANG_AR_KW				0x3401 /* Arabic (Kuwait) */
#define	USB_LANG_AR_AE				0x3801 /* Arabic (U.A.E.) */
#define	USB_LANG_AR_BH				0x3c01 /* Arabic (Bahrain) */
#define	USB_LANG_AR_QA				0x4001 /* Arabic (Qatar) */
#define	USB_LANG_HY					0x042b /* Armenian */
#define	USB_LANG_AS					0x044d /* Assamese */
#define	USB_LANG_AZ_AZ_LATIN		0x042c /* Azeri (Latin) */
#define	USB_LANG_AZ_AZ				0x082c /* Azeri (Cyrillic) */
#define	USB_LANG_EU					0x042d /* Basque */
#define	USB_LANG_BE_BY				0x0423 /* Belarussian */
#define	USB_LANG_BN					0x0445 /* Bengali */
#define	USB_LANG_BG					0x0402 /* Bulgarian */
#define	USB_LANG_MY_MM				0x0455 /* Burmese */
#define	USB_LANG_CA					0x0403 /* Catalan */
#define	USB_LANG_ZH_TW				0x0404 /* Chinese (Taiwan) */
#define	USB_LANG_ZH_CN				0x0804 /* Chinese (PRC) */
#define	USB_LANG_ZH_HK				0x0c04 /* Chinese (Hong Kong SAR, PRC) */
#define	USB_LANG_ZH_SG				0x1004 /* Chinese (Singapore) */
#define	USB_LANG_ZH_MO				0x1404 /* Chinese (Macau SAR) */
#define	USB_LANG_HR					0x041a /* Croatian */
#define	USB_LANG_CS					0x0405 /* Czech */
#define	USB_LANG_DA					0x0406 /* Danish */
#define	USB_LANG_NL_NL				0x0413 /* Dutch (Netherlands) */
#define	USB_LANG_NL_BE				0x0813 /* Dutch (Belgium) */
#define	USB_LANG_EN_US				0x0409 /* English (United States) */
#define	USB_LANG_EN_UK				0x0809 /* English (United Kingdom) */
#define	USB_LANG_EN_AU				0x0c09 /* English (Australian) */
#define	USB_LANG_EN_CA				0x1009 /* English (Canadian) */
#define	USB_LANG_EN_NZ				0x1409 /* English (New Zealand) */
#define	USB_LANG_EN_IE				0x1809 /* English (Ireland) */
#define	USB_LANG_EN_ZA				0x1c09 /* English (South Africa) */
#define	USB_LANG_EN_JM				0x2009 /* English (Jamaica) */
#define	USB_LANG_EN_VI				0x2409 /* English (Caribbean) */
#define	USB_LANG_EN_BZ				0x2809 /* English (Belize) */
#define	USB_LANG_EN_TT				0x2c09 /* English (Trinidad) */
#define	USB_LANG_EN_ZW				0x3009 /* English (Zimbabwe) */
#define	USB_LANG_EN_PH				0x3409 /* English (Philippines) */
#define	USB_LANG_ET					0x0425 /* Estonian */
#define	USB_LANG_FO					0x0438 /* Faeroese */
#define	USB_LANG_FA					0x0429 /* Farsi */
#define	USB_LANG_FI					0x040b /* Finnish */
#define	USB_LANG_FR_FR				0x040c /* French (Standard) */
#define	USB_LANG_FR_BE				0x080c /* French (Belgian) */
#define	USB_LANG_FR_CA				0x0c0c /* French (Canadian) */
#define	USB_LANG_FR_CH				0x100c /* French (Switzerland) */
#define	USB_LANG_FR_LU				0x140c /* French (Luxembourg) */
#define	USB_LANG_FR_MC				0x180c /* French (Monaco) */
#define	USB_LANG_KA					0x0437 /* Georgian */
#define	USB_LANG_DE_DE				0x0407 /* German (Standard) */
#define	USB_LANG_DE_CH				0x0807 /* German (Switzerland) */
#define	USB_LANG_DE_AS				0x0c07 /* German (Austria) */
#define	USB_LANG_DE_LU				0x1007 /* German (Luxembourg) */
#define	USB_LANG_DE_LI				0x1407 /* German (Liechtenstein) */
#define	USB_LANG_EL					0x0408 /* Greek */
#define	USB_LANG_GU					0x0447 /* Gujarati */
#define	USB_LANG_HE					0x040d /* Hebrew */
#define	USB_LANG_HI					0x0439 /* Hindi */
#define	USB_LANG_HU					0x040e /* Hungarian */
#define	USB_LANG_IS					0x040f /* Icelandic */
#define	USB_LANG_IN					0x0421 /* Indonesian */
#define	USB_LANG_IT					0x0410 /* Italian (Standard) */
#define	USB_LANG_IT_CH				0x0810 /* Italian (Switzerland) */
#define	USB_LANG_JP					0x0411 /* Japanese */
#define	USB_LANG_KN					0x044b /* Kannada */
#define	USB_LANG_KS_IN				0x0860 /* Kashmiri (India) */
#define	USB_LANG_KK					0x043f /* Kazakh */
#define	USB_LANG_KOK				0x0457 /* Konkani */
#define	USB_LANG_KO_KR				0x0412 /* Korean */
#define	USB_LANG_KO					0x0812 /* Korean (Johab) */
#define	USB_LANG_LV					0x0426 /* Latvian */
#define	USB_LANG_LT_LT				0x0427 /* Lithuanian */
#define	USB_LANG_LT					0x0827 /* Lithuanian (Classic) */
#define	USB_LANG_MK					0x042f /* Macedonian */
#define	USB_LANG_MS_MY				0x043e /* Malay (Malaysian) */
#define	USB_LANG_MS_BN				0x083e /* Malay (Brunei Darussalam) */
#define	USB_LANG_ML					0x044c /* Malayalam */
#define	USB_LANG_MNI				0x0458 /* Manipuri */
#define	USB_LANG_MR					0x044e /* Marathi */
#define	USB_LANG_NE_IN				0x0861 /* Nepali (India) */
#define	USB_LANG_NB_NO				0x0414 /* Norwegian (Bokmal) */
#define	USB_LANG_NN_NO				0x0814 /* Norwegian (Nynorsk) */
#define	USB_LANG_OR					0x0448 /* Oriya */
#define	USB_LANG_PL					0x0415 /* Polish */
#define	USB_LANG_PT_BR				0x0416 /* Portuguese (Brazil) */
#define	USB_LANG_PT_PT				0x0816 /* Portuguese (Standard) */
#define	USB_LANG_PA					0x0446 /* Punjabi */
#define	USB_LANG_RO					0x0418 /* Romanian */
#define	USB_LANG_RU					0x0419 /* Russian */
#define	USB_LANG_SA					0x044f /* Sanskrit */
#define	USB_LANG_SR					0x0c1a /* Serbian (Cyrillic) */
#define	USB_LANG_SR_LATIN			0x081a /* Serbian (Latin) */
#define	USB_LANG_SD					0x0459 /* Sindhi */
#define	USB_LANG_SK					0x041b /* Slovak */
#define	USB_LANG_SL					0x0424 /* Slovenian */
#define	USB_LANG_ES					0x040a /* Spanish (Traditional Sort) */
#define	USB_LANG_ES_MX				0x080a /* Spanish (Mexican) */
#define	USB_LANG_ES_ES				0x0c0a /* Spanish (Modern Sort) */
#define	USB_LANG_ES_GT				0x100a /* Spanish (Guatemala) */
#define	USB_LANG_ES_CR				0x140a /* Spanish (Costa Rica) */
#define	USB_LANG_ES_PA				0x180a /* Spanish (Panama) */
#define	USB_LANG_ES_DO				0x1c0a /* Spanish (Dominican Republic) */
#define	USB_LANG_ES_VE				0x200a /* Spanish (Venezuela) */
#define	USB_LANG_ES_CO				0x240a /* Spanish (Colombia) */
#define	USB_LANG_ES_PE				0x280a /* Spanish (Peru) */
#define	USB_LANG_ES_AR				0x2c0a /* Spanish (Argentina) */
#define	USB_LANG_ES_EC				0x300a /* Spanish (Ecuador) */
#define	USB_LANG_ES_CL				0x340a /* Spanish (Chile) */
#define	USB_LANG_ES_UY				0x380a /* Spanish (Uruguay) */
#define	USB_LANG_ES_PY				0x3c0a /* Spanish (Paraguay) */
#define	USB_LANG_ES_BO				0x400a /* Spanish (Bolivia) */
#define	USB_LANG_ES_SV				0x440a /* Spanish (El Salvador) */
#define	USB_LANG_ES_HN				0x480a /* Spanish (Honduras) */
#define	USB_LANG_ES_NI				0x4c0a /* Spanish (Nicaragua) */
#define	USB_LANG_ES_PR				0x500a /* Spanish (Puerto Rico) */
#define	USB_LANG_NGO				0x0430 /* Sutu */
#define	USB_LANG_SW_KE				0x0441 /* Swahili (Kenya) */
#define	USB_LANG_SE					0x041d /* Swedish */
#define	USB_LANG_SE_FI				0x081d /* Swedish (Finland) */
#define	USB_LANG_TA					0x0449 /* Tamil */
#define	USB_LANG_TT					0x0444 /* Tatar (Tatarstan) */
#define	USB_LANG_TE					0x044a /* Telugu */
#define	USB_LANG_TH					0x041e /* Thai */
#define	USB_LANG_TR					0x041f /* Turkish */
#define	USB_LANG_UK					0x0422 /* Ukrainian */
#define	USB_LANG_UR_PK				0x0420 /* Urdu (Pakistan) */
#define	USB_LANG_UR_IN				0x0820 /* Urdu (India) */
#define	USB_LANG_UZ_UZ_LATIN		0x0443 /* Uzbek (Latin) */
#define	USB_LANG_UZ_UZ				0x0843 /* Uzbek (Cyrillic) */
#define	USB_LANG_VI					0x042a /* Vietnamese */

/*
 * USB Primary Language Identifers
 */
#define USB_PLANG_NONE				0x00 /* None */
#define USB_PLANG_ARABIC			0x01 /* Arabic */
#define USB_PLANG_BULGARIAN			0x02 /* Bulgarian */
#define USB_PLANG_CATALAN			0x03 /* Catalan */
#define USB_PLANG_CHINESE			0x04 /* Chinese */
#define USB_PLANG_CZECH				0x05 /* Czech */
#define USB_PLANG_DANISH			0x06 /* Danish */
#define USB_PLANG_GERMAN			0x07 /* German */
#define USB_PLANG_GREEK				0x08 /* Greek */
#define USB_PLANG_ENGLISH			0x09 /* English */
#define USB_PLANG_SPANISH			0x0a /* Spanish */
#define USB_PLANG_FINNISH			0x0b /* Finnish */
#define USB_PLANG_FRENCH			0x0c /* French */
#define USB_PLANG_HEBREW			0x0d /* Hebrew */
#define USB_PLANG_HUNGARIAN			0x0e /* Hungarian */
#define USB_PLANG_ICELANDIC			0x0f /* Icelandic */
#define USB_PLANG_ITALIAN			0x10 /* Italian */
#define USB_PLANG_JAPANESE			0x11 /* Japanese */
#define USB_PLANG_KOREAN			0x12 /* Korean */
#define USB_PLANG_DUTCH				0x13 /* Dutch */
#define USB_PLANG_NORWEGIAN			0x14 /* Norwegian */
#define USB_PLANG_POLISH			0x15 /* Polish */
#define USB_PLANG_PORTUGUESE		0x16 /* Portuguese */
#define USB_PLANG_ROMANIAN			0x18 /* Romanian */
#define USB_PLANG_RUSSIAN			0x19 /* Russian */
#define USB_PLANG_CROATIAN			0x1a /* Croatian */
#define USB_PLANG_SERBIAN			0x1a /* Serbian */
#define USB_PLANG_SLOVAK			0x1b /* Slovak */
#define USB_PLANG_ALBANIAN			0x1c /* Albanian */
#define USB_PLANG_SWEDISH			0x1d /* Swedish */
#define USB_PLANG_THAI				0x1e /* Thai */
#define USB_PLANG_TURKISH			0x1f /* Turkish */
#define USB_PLANG_URDU				0x20 /* Urdu */
#define USB_PLANG_INDONESIAN		0x21 /* Indonesian */
#define USB_PLANG_UKRANIAN			0x22 /* Ukrainian */
#define USB_PLANG_BELARUSIAN		0x23 /* Belarusian */
#define USB_PLANG_SLOVENIAN			0x24 /* Slovenian */
#define USB_PLANG_ESTONIAN			0x25 /* Estonian */
#define USB_PLANG_LATVIAN			0x26 /* Latvian */
#define USB_PLANG_LITHUANIAN		0x27 /* Lithuanian */
#define USB_PLANG_FARSI				0x29 /* Farsi */
#define USB_PLANG_VIETNAMESE		0x2a /* Vietnamese */
#define USB_PLANG_ARMENIAN			0x2b /* Armenian */
#define USB_PLANG_AZERI				0x2c /* Azeri */
#define USB_PLANG_BASQUE			0x2d /* Basque */
#define USB_PLANG_MACEDONIAN		0x2f /* Macedonian */
#define USB_PLANG_AFRIKAANS			0x36 /* Afrikaans */
#define USB_PLANG_GEORGIAN			0x37 /* Georgian */
#define USB_PLANG_FAEROESE			0x38 /* Faeroese */
#define USB_PLANG_HINDI				0x39 /* Hindi */
#define USB_PLANG_MALAY				0x3e /* Malay */
#define USB_PLANG_KAZAK				0x3f /* Kazak */
#define USB_PLANG_SWAHILI			0x41 /* Swahili */
#define USB_PLANG_UZBEK				0x43 /* Uzbek */
#define USB_PLANG_TATAR				0x44 /* Tatar */
#define USB_PLANG_BENGALI			0x45 /* Bengali */
#define USB_PLANG_PUNJABI			0x46 /* Punjabi */
#define USB_PLANG_GUJARATI			0x47 /* Gujarati */
#define USB_PLANG_ORIYA				0x48 /* Oriya */
#define USB_PLANG_TAMIL				0x49 /* Tamil */
#define USB_PLANG_TELUGU			0x4a /* Telugu */
#define USB_PLANG_KANNADA			0x4b /* Kannada */
#define USB_PLANG_MALAYALAM			0x4c /* Malayalam */
#define USB_PLANG_ASSAMESE			0x4d /* Assamese */
#define USB_PLANG_MARATHI			0x4e /* Marathi */
#define USB_PLANG_SANSKRIT			0x4f /* Sanskrit */
#define USB_PLANG_KONKANI			0x57 /* Konkani */
#define USB_PLANG_MANIPURI			0x58 /* Manipuri */
#define USB_PLANG_SINDHI			0x59 /* Sindhi */
#define USB_PLANG_KASHMIRI			0x60 /* Kashmiri */
#define USB_PLANG_NEPALI			0x61 /* Nepali */
#define USB_PLANG_HID				0xff /* HID */

/*
 * USB Sublanguage Idenfiers
 */
#define	USB_SLANG_AR_SA				0x01 /* Arabic (Saudi Arabia) */
#define	USB_SLANG_AR_IQ				0x02 /* Arabic (Iraq) */
#define	USB_SLANG_AR_EG				0x03 /* Arabic (Egypt) */
#define	USB_SLANG_AR_LY				0x04 /* Arabic (Libya) */
#define	USB_SLANG_AR_DZ				0x05 /* Arabic (Algeria) */
#define	USB_SLANG_AR_MA				0x06 /* Arabic (Morocco) */
#define	USB_SLANG_AR_TN				0x07 /* Arabic (Tunisia) */
#define	USB_SLANG_AR_OM				0x08 /* Arabic (Oman) */
#define	USB_SLANG_AR_YE				0x09 /* Arabic (Yemen) */
#define	USB_SLANG_AR_SY				0x10 /* Arabic (Syria) */
#define	USB_SLANG_AR_JO				0x11 /* Arabic (Jordan) */
#define	USB_SLANG_AR_LB				0x12 /* Arabic (Lebanon) */
#define	USB_SLANG_AR_KW				0x13 /* Arabic (Kuwait) */
#define	USB_SLANG_AR_AE				0x14 /* Arabic (U.A.E.) */
#define	USB_SLANG_AR_BH				0x15 /* Arabic (Bahrain) */
#define	USB_SLANG_AR_QA				0x16 /* Arabic (Qatar) */
#define	USB_SLANG_AZ_AZ				0x01 /* Azeri (Cyrillic) */
#define	USB_SLANG_AZ_AZ_LATIN		0x02 /* Azeri (Latin) */
#define	USB_SLANG_ZH_TW				0x01 /* Chinese (Traditional) */
#define	USB_SLANG_ZH_CN				0x02 /* Chinese (Simplified) */
#define	USB_SLANG_ZH_HK				0x03 /* Chinese (Hong Kong SAR, PRC) */
#define	USB_SLANG_ZH_SG				0x04 /* Chinese (Singapore) */
#define	USB_SLANG_ZH_MO				0x05 /* Chinese (Macau SAR) */
#define	USB_SLANG_NL				0x01 /* Dutch */
#define	USB_SLANG_NL_BE				0x02 /* Dutch (Belgian) */
#define	USB_SLANG_EN_US				0x01 /* English (US) */
#define	USB_SLANG_EN_UK				0x02 /* English (UK) */
#define	USB_SLANG_EN_AU				0x03 /* English (Australian) */
#define	USB_SLANG_EN_CA				0x04 /* English (Canadian) */
#define	USB_SLANG_EN_NZ				0x05 /* English (New Zealand) */
#define	USB_SLANG_EN_IE				0x06 /* English (Ireland) */
#define	USB_SLANG_EN_ZA				0x07 /* English (South Africa) */
#define	USB_SLANG_EN_JM				0x08 /* English (Jamaica) */
#define	USB_SLANG_EN_VI				0x09 /* English (Caribbean) */
#define	USB_SLANG_EN_BZ				0x0a /* English (Belize) */
#define	USB_SLANG_EN_TT				0x0b /* English (Trinidad) */
#define	USB_SLANG_EN_PH				0x0c /* English (Philippines) */
#define	USB_SLANG_EN_ZW				0x0d /* English (Zimbabwe) */
#define	USB_SLANG_FR				0x01 /* French */
#define	USB_SLANG_FR_BE				0x02 /* French (Belgian) */
#define	USB_SLANG_FR_CA				0x03 /* French (Canadian) */
#define	USB_SLANG_FR_CH				0x04 /* French (Swiss) */
#define	USB_SLANG_FR_LU				0x05 /* French (Luxembourg) */
#define	USB_SLANG_FR_MC				0x06 /* French (Monaco) */
#define	USB_SLANG_DE				0x01 /* German */
#define	USB_SLANG_DE_CH				0x02 /* German (Swiss) */
#define	USB_SLANG_DE_AS				0x03 /* German (Austrian) */
#define	USB_SLANG_DE_LU				0x04 /* German (Luxembourg) */
#define	USB_SLANG_DE_LI				0x05 /* German (Liechtenstein) */
#define	USB_SLANG_IT				0x01 /* Italian */
#define	USB_SLANG_IT_CH				0x02 /* Italian (Swiss) */
#define	USB_SLANG_KS_IN				0x02 /* Kashmiri (India) */
#define	USB_SLANG_KO				0x01 /* Korean */
#define	USB_SLANG_LT				0x01 /* Lithuanian */
#define	USB_SLANG_MS_MY				0x01 /* Malay (Malaysia) */
#define	USB_SLANG_MS_BN				0x02 /* Malay (Brunei Darassalam) */
#define	USB_SLANG_NE_IN				0x02 /* Nepali (India) */
#define	USB_SLANG_NB_NO				0x01 /* Norwegian (Bokmal) */
#define	USB_SLANG_NN_NO				0x02 /* Norwegian (Nynorsk) */
#define	USB_SLANG_PT				0x01 /* Portuguese (Brazilian) */
#define	USB_SLANG_PT_BR				0x02 /* Portuguese */
#define	USB_SLANG_SR_SR_LATIN		0x02 /* Serbian (Latin) */
#define	USB_SLANG_SR_SR				0x03 /* Serbian (Cyrillic) */
#define	USB_SLANG_ES				0x01 /* Spanish (Castilian) */
#define	USB_SLANG_ES_MX				0x02 /* Spanish (Mexican) */
#define	USB_SLANG_ES_ES				0x03 /* Spanish (Modern) */
#define	USB_SLANG_ES_GT				0x04 /* Spanish (Guatemala) */
#define	USB_SLANG_ES_CR				0x05 /* Spanish (Costa Rica) */
#define	USB_SLANG_ES_PA				0x06 /* Spanish (Panama) */
#define	USB_SLANG_ES_DO				0x07 /* Spanish (Dominican Republic) */
#define	USB_SLANG_ES_VE				0x08 /* Spanish (Venezuela) */
#define	USB_SLANG_ES_CO				0x09 /* Spanish (Colombia) */
#define	USB_SLANG_ES_PE				0x0a /* Spanish (Peru) */
#define	USB_SLANG_ES_AR				0x0b /* Spanish (Argentina) */
#define	USB_SLANG_ES_EC				0x0c /* Spanish (Ecuador) */
#define	USB_SLANG_ES_CL				0x0d /* Spanish (Chile) */
#define	USB_SLANG_ES_UY				0x0e /* Spanish (Uruguay) */
#define	USB_SLANG_ES_PY				0x0f /* Spanish (Paraguay) */
#define	USB_SLANG_ES_BO				0x10 /* Spanish (Bolivia) */
#define	USB_SLANG_ES_SV				0x11 /* Spanish (El Salvador) */
#define	USB_SLANG_ES_HN				0x12 /* Spanish (Honduras) */
#define	USB_SLANG_ES_NI				0x13 /* Spanish (Nicaragua) */
#define	USB_SLANG_ES_PR				0x14 /* Spanish (Puerto Rico) */
#define	USB_SLANG_SE				0x01 /* Swedish */
#define	USB_SLANG_SE_FI				0x02 /* Swedish (Finland) */
#define	USB_SLANG_UR_PK				0x01 /* Urdu (Pakistan) */
#define	USB_SLANG_UR_IN				0x02 /* Urdu (India) */
#define	USB_SLANG_UZ_UZ_LATIN		0x01 /* Uzbek (Latin) */
#define	USB_SLANG_UZ_UZ				0x02 /* Uzbek (Cyrillic) */

/* USB_DT_STRING: String descriptor */
struct usb_string_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	union {					/* UTF-16LE encoded */
		__le16 bString[1];
		__le16 wLangId[1];
		__le16 wData[1];
	};
} __attribute__ ((packed));

/* note that "string" zero is special, it holds language codes that
 * the device supports, not Unicode characters.
 */

/*-------------------------------------------------------------------------*/

/* USB_DT_INTERFACE: Interface descriptor */
struct usb_interface_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bInterfaceNumber;
	__u8  bAlternateSetting;
	__u8  bNumEndpoints;
	__u8  bInterfaceClass;
	__u8  bInterfaceSubClass;
	__u8  bInterfaceProtocol;
	__u8  iInterface;
} __attribute__ ((packed));

#define USB_DT_INTERFACE_SIZE		9

/*-------------------------------------------------------------------------*/

/* USB_DT_ENDPOINT: Endpoint descriptor */
struct usb_endpoint_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bEndpointAddress;
	__u8  bmAttributes;
	__le16 wMaxPacketSize;
	__u8  bInterval;

	/* NOTE:  these two are _only_ in audio endpoints. */
	/* use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof. */
	__u8  bRefresh;
	__u8  bSynchAddress;
} __attribute__ ((packed));

#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */


/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_SYNCTYPE		0x0c
#define USB_ENDPOINT_SYNC_NONE		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE	(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC		(3 << 2)

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80

/*-------------------------------------------------------------------------*/

/**
 * usb_endpoint_num - get the endpoint's number
 * @epd: endpoint to be checked
 *
 * Returns @epd's number: 0 to 15.
 */
static inline int usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

/**
 * usb_endpoint_type - get the endpoint's transfer type
 * @epd: endpoint to be checked
 *
 * Returns one of USB_ENDPOINT_XFER_{CONTROL, ISOC, BULK, INT} according
 * to @epd's transfer type.
 */
static inline int usb_endpoint_type(const struct usb_endpoint_descriptor *epd)
{
	return epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

/**
 * usb_endpoint_dir_in - check if the endpoint has IN direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type IN, otherwise it returns false.
 */
static inline int usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

/**
 * usb_endpoint_dir_out - check if the endpoint has OUT direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type OUT, otherwise it returns false.
 */
static inline int usb_endpoint_dir_out(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

/**
 * usb_endpoint_xfer_bulk - check if the endpoint has bulk transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type bulk, otherwise it returns false.
 */
static inline int usb_endpoint_xfer_bulk(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_BULK);
}

/**
 * usb_endpoint_xfer_control - check if the endpoint has control transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type control, otherwise it returns false.
 */
static inline int usb_endpoint_xfer_control(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_CONTROL);
}

/**
 * usb_endpoint_xfer_int - check if the endpoint has interrupt transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type interrupt, otherwise it returns
 * false.
 */
static inline int usb_endpoint_xfer_int(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_INT);
}

/**
 * usb_endpoint_xfer_isoc - check if the endpoint has isochronous transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type isochronous, otherwise it returns
 * false.
 */
static inline int usb_endpoint_xfer_isoc(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_ISOC);
}

/**
 * usb_endpoint_is_bulk_in - check if the endpoint is bulk IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has bulk transfer type and IN direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_bulk_in(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_in(epd);
}

/**
 * usb_endpoint_is_bulk_out - check if the endpoint is bulk OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has bulk transfer type and OUT direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_bulk_out(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_out(epd);
}

/**
 * usb_endpoint_is_int_in - check if the endpoint is interrupt IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has interrupt transfer type and IN direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_int_in(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_in(epd);
}

/**
 * usb_endpoint_is_int_out - check if the endpoint is interrupt OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has interrupt transfer type and OUT direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_int_out(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_out(epd);
}

/**
 * usb_endpoint_is_isoc_in - check if the endpoint is isochronous IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has isochronous transfer type and IN direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_isoc_in(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_in(epd);
}

/**
 * usb_endpoint_is_isoc_out - check if the endpoint is isochronous OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has isochronous transfer type and OUT direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_isoc_out(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_out(epd);
}

/*-------------------------------------------------------------------------*/

/* USB_DT_SS_ENDPOINT_COMP: SuperSpeed Endpoint Companion descriptor */
struct usb_ss_ep_comp_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bMaxBurst;
	__u8  bmAttributes;
	__u16 wBytesPerInterval;
} __attribute__ ((packed));

#define USB_DT_SS_EP_COMP_SIZE		6
/* Bits 4:0 of bmAttributes if this is a bulk endpoint */
#define USB_SS_MAX_STREAMS(p)		(1 << (p & 0x1f))

/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE_QUALIFIER: Device Qualifier descriptor */
struct usb_qualifier_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__le16 bcdUSB;
	__u8  bDeviceClass;
	__u8  bDeviceSubClass;
	__u8  bDeviceProtocol;
	__u8  bMaxPacketSize0;
	__u8  bNumConfigurations;
	__u8  bRESERVED;
} __attribute__ ((packed));


/*-------------------------------------------------------------------------*/

/* USB_DT_OTG (from OTG 1.0a supplement) */
struct usb_otg_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bmAttributes;	/* support for HNP, SRP, etc */
} __attribute__ ((packed));

/* from usb_otg_descriptor.bmAttributes */
#define USB_OTG_SRP		(1 << 0)
#define USB_OTG_HNP		(1 << 1)	/* swap host/device roles */

/*-------------------------------------------------------------------------*/

/* USB_DT_DEBUG:  for special highspeed devices, replacing serial console */
struct usb_debug_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	/* bulk endpoints with 8 byte maxpacket */
	__u8  bDebugInEndpoint;
	__u8  bDebugOutEndpoint;
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_DT_INTERFACE_ASSOCIATION: groups interfaces */
struct usb_interface_assoc_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bFirstInterface;
	__u8  bInterfaceCount;
	__u8  bFunctionClass;
	__u8  bFunctionSubClass;
	__u8  bFunctionProtocol;
	__u8  iFunction;
} __attribute__ ((packed));


/*-------------------------------------------------------------------------*/

/* USB_DT_SECURITY:  group of wireless security descriptors, including
 * encryption types available for setting up a CC/association.
 */
struct usb_security_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__le16 wTotalLength;
	__u8  bNumEncryptionTypes;
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_DT_KEY:  used with {GET,SET}_SECURITY_DATA; only public keys
 * may be retrieved.
 */
struct usb_key_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  tTKID[3];
	__u8  bReserved;
	__u8  bKeyData[0];
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_DT_ENCRYPTION_TYPE:  bundled in DT_SECURITY groups */
struct usb_encryption_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bEncryptionType;
#define	USB_ENC_TYPE_UNSECURE		0
#define	USB_ENC_TYPE_WIRED		1	/* non-wireless mode */
#define	USB_ENC_TYPE_CCM_1		2	/* aes128/cbc session */
#define	USB_ENC_TYPE_RSA_1		3	/* rsa3072/sha1 auth */
	__u8  bEncryptionValue;		/* use in SET_ENCRYPTION */
	__u8  bAuthKeyIndex;
} __attribute__((packed));


/*-------------------------------------------------------------------------*/

/* USB_DT_BOS:  group of device-level capabilities */
struct usb_bos_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__le16 wTotalLength;
	__u8  bNumDeviceCaps;
} __attribute__((packed));

#define USB_DT_BOS_SIZE		5
/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE_CAPABILITY:  grouped with BOS */
struct usb_dev_cap_header {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
} __attribute__((packed));

#define	USB_CAP_TYPE_WIRELESS_USB	1

struct usb_wireless_cap_descriptor {	/* Ultra Wide Band */
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;

	__u8  bmAttributes;
#define	USB_WIRELESS_P2P_DRD		(1 << 1)
#define	USB_WIRELESS_BEACON_MASK	(3 << 2)
#define	USB_WIRELESS_BEACON_SELF	(1 << 2)
#define	USB_WIRELESS_BEACON_DIRECTED	(2 << 2)
#define	USB_WIRELESS_BEACON_NONE	(3 << 2)
	__le16 wPHYRates;	/* bit rates, Mbps */
#define	USB_WIRELESS_PHY_53		(1 << 0)	/* always set */
#define	USB_WIRELESS_PHY_80		(1 << 1)
#define	USB_WIRELESS_PHY_107		(1 << 2)	/* always set */
#define	USB_WIRELESS_PHY_160		(1 << 3)
#define	USB_WIRELESS_PHY_200		(1 << 4)	/* always set */
#define	USB_WIRELESS_PHY_320		(1 << 5)
#define	USB_WIRELESS_PHY_400		(1 << 6)
#define	USB_WIRELESS_PHY_480		(1 << 7)
	__u8  bmTFITXPowerInfo;	/* TFI power levels */
	__u8  bmFFITXPowerInfo;	/* FFI power levels */
	__le16 bmBandGroup;
	__u8  bReserved;
} __attribute__((packed));

/* USB 2.0 Extension descriptor */
#define	USB_CAP_TYPE_EXT		2

struct usb_ext_cap_descriptor {		/* Link Power Management */
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
	__le32 bmAttributes;
#define USB_LPM_SUPPORT			(1 << 1)	/* supports LPM */
} __attribute__((packed));

#define USB_DT_USB_EXT_CAP_SIZE	7

/*
 * SuperSpeed USB Capability descriptor: Defines the set of SuperSpeed USB
 * specific device level capabilities
 */
#define		USB_SS_CAP_TYPE		3
struct usb_ss_cap_descriptor {		/* Link Power Management */
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
	__u8  bmAttributes;
#define USB_LTM_SUPPORT			(1 << 1) /* supports LTM */
	__le16 wSpeedSupported;
#define USB_LOW_SPEED_OPERATION		(1)	 /* Low speed operation */
#define USB_FULL_SPEED_OPERATION	(1 << 1) /* Full speed operation */
#define USB_HIGH_SPEED_OPERATION	(1 << 2) /* High speed operation */
#define USB_5GBPS_OPERATION		(1 << 3) /* Operation at 5Gbps */
	__u8  bFunctionalitySupport;
	__u8  bU1devExitLat;
	__le16 bU2DevExitLat;
} __attribute__((packed));

#define USB_DT_USB_SS_CAP_SIZE	10

/*
 * Container ID Capability descriptor: Defines the instance unique ID used to
 * identify the instance across all operating modes
 */
#define	CONTAINER_ID_TYPE	4
struct usb_ss_container_id_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
	__u8  bReserved;
	__u8  ContainerID[16]; /* 128-bit number */
} __attribute__((packed));

#define USB_DT_USB_SS_CONTN_ID_SIZE	20
/*-------------------------------------------------------------------------*/

/* USB_DT_WIRELESS_ENDPOINT_COMP:  companion descriptor associated with
 * each endpoint descriptor for a wireless device
 */
struct usb_wireless_ep_comp_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bMaxBurst;
	__u8  bMaxSequence;
	__le16 wMaxStreamDelay;
	__le16 wOverTheAirPacketSize;
	__u8  bOverTheAirInterval;
	__u8  bmCompAttributes;
#define USB_ENDPOINT_SWITCH_MASK	0x03	/* in bmCompAttributes */
#define USB_ENDPOINT_SWITCH_NO		0
#define USB_ENDPOINT_SWITCH_SWITCH	1
#define USB_ENDPOINT_SWITCH_SCALE	2
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_REQ_SET_HANDSHAKE is a four-way handshake used between a wireless
 * host and a device for connection set up, mutual authentication, and
 * exchanging short lived session keys.  The handshake depends on a CC.
 */
struct usb_handshake {
	__u8 bMessageNumber;
	__u8 bStatus;
	__u8 tTKID[3];
	__u8 bReserved;
	__u8 CDID[16];
	__u8 nonce[16];
	__u8 MIC[8];
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_REQ_SET_CONNECTION modifies or revokes a connection context (CC).
 * A CC may also be set up using non-wireless secure channels (including
 * wired USB!), and some devices may support CCs with multiple hosts.
 */
struct usb_connection_context {
	__u8 CHID[16];		/* persistent host id */
	__u8 CDID[16];		/* device id (unique w/in host context) */
	__u8 CK[16];		/* connection key */
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB 2.0 defines three speeds, here's how Linux identifies them */

enum usb_device_speed {
	USB_SPEED_UNKNOWN = 0,			/* enumerating */
	USB_SPEED_LOW, USB_SPEED_FULL,		/* usb 1.1 */
	USB_SPEED_HIGH,				/* usb 2.0 */
	USB_SPEED_WIRELESS,			/* wireless (usb 2.5) */
	USB_SPEED_SUPER,			/* usb 3.0 */
};

enum usb_device_state {
	/* NOTATTACHED isn't in the USB spec, and this state acts
	 * the same as ATTACHED ... but it's clearer this way.
	 */
	USB_STATE_NOTATTACHED = 0,

	/* chapter 9 and authentication (wireless) device states */
	USB_STATE_ATTACHED,
	USB_STATE_POWERED,			/* wired */
	USB_STATE_RECONNECTING,			/* auth */
	USB_STATE_UNAUTHENTICATED,		/* auth */
	USB_STATE_DEFAULT,			/* limited function */
	USB_STATE_ADDRESS,
	USB_STATE_CONFIGURED,			/* most functions */

	USB_STATE_SUSPENDED

	/* NOTE:  there are actually four different SUSPENDED
	 * states, returning to POWERED, DEFAULT, ADDRESS, or
	 * CONFIGURED respectively when SOF tokens flow again.
	 * At this level there's no difference between L1 and L2
	 * suspend states.  (L2 being original USB 1.1 suspend.)
	 */
};

/*-------------------------------------------------------------------------*/

/*
 * As per USB compliance update, a device that is actively drawing
 * more than 100mA from USB must report itself as bus-powered in
 * the GetStatus(DEVICE) call.
 * http://compliance.usb.org/index.asp?UpdateFile=Electrical&Format=Standard#34
 */
#define USB_SELF_POWER_VBUS_MAX_DRAW		100

#endif /* __LINUX_USB_CH9_H */
