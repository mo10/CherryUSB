#include "usbd_core.h"
#include "usbd_msc.h"
#include "usbd_cdc.h"
#include "usbd_hid.h"

/*!< endpoint address */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define MSC_IN_EP  0x84
#define MSC_OUT_EP 0x05

/*!< endpoint address */
#define HID_INT_EP          0x86
#define HID_INT_EP_SIZE     8
#define HID_INT_EP_INTERVAL 10

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

/*!< report descriptor size */
#define HID_MOUSE_REPORT_DESC_SIZE 74

#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN + MSC_DESCRIPTOR_LEN + 25)

const uint8_t cdc_acm_hid_msc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x04, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 0x02),
    MSC_DESCRIPTOR_INIT(0x02, MSC_OUT_EP, MSC_IN_EP, 0x02),
    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09,                          /* bLength: Interface Descriptor size */
    USB_DESCRIPTOR_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
    0x03,                          /* bInterfaceNumber: Number of Interface */
    0x00,                          /* bAlternateSetting: Alternate setting */
    0x01,                          /* bNumEndpoints */
    0x03,                          /* bInterfaceClass: HID */
    0x01,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
    0x02,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
    0,                             /* iInterface: Index of string descriptor */
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09,                    /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE_HID, /* bDescriptorType: HID */
    0x11,                    /* bcdHID: HID Class Spec release number */
    0x01,
    0x00,                       /* bCountryCode: Hardware target country */
    0x01,                       /* bNumDescriptors: Number of HID class descriptors to follow */
    0x22,                       /* bDescriptorType */
    HID_MOUSE_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
    0x00,
    /******************** Descriptor of Mouse endpoint ********************/
    /* 27 */
    0x07,                         /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
    HID_INT_EP,                   /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                         /* bmAttributes: Interrupt endpoint */
    HID_INT_EP_SIZE,              /* wMaxPacketSize: 4 Byte max */
    0x00,
    HID_INT_EP_INTERVAL, /* bInterval: Polling Interval */
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'C', 0x00,                  /* wcChar10 */
    'M', 0x00,                  /* wcChar11 */
    'H', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};

#define BLOCK_SIZE  512
#define BLOCK_COUNT 10

typedef struct
{
    uint8_t BlockSpace[BLOCK_SIZE];
} BLOCK_TYPE;

BLOCK_TYPE mass_block[BLOCK_COUNT];

void usbd_msc_get_cap(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    *block_num = 1000; //Pretend having so many buffer,not has actually.
    *block_size = BLOCK_SIZE;
}
int usbd_msc_sector_read(uint32_t sector, uint8_t *buffer, uint32_t length)
{
    if (sector < 10)
        memcpy(buffer, mass_block[sector].BlockSpace, length);
    return 0;
}

int usbd_msc_sector_write(uint32_t sector, uint8_t *buffer, uint32_t length)
{
    if (sector < 10)
        memcpy(mass_block[sector].BlockSpace, buffer, length);
    return 0;
}

/*!< hid mouse report descriptor */
static const uint8_t hid_mouse_report_desc[HID_MOUSE_REPORT_DESC_SIZE] = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x02, // USAGE (Mouse)
    0xA1, 0x01, // COLLECTION (Application)
    0x09, 0x01, //   USAGE (Pointer)

    0xA1, 0x00, //   COLLECTION (Physical)
    0x05, 0x09, //     USAGE_PAGE (Button)
    0x19, 0x01, //     USAGE_MINIMUM (Button 1)
    0x29, 0x03, //     USAGE_MAXIMUM (Button 3)

    0x15, 0x00, //     LOGICAL_MINIMUM (0)
    0x25, 0x01, //     LOGICAL_MAXIMUM (1)
    0x95, 0x03, //     REPORT_COUNT (3)
    0x75, 0x01, //     REPORT_SIZE (1)

    0x81, 0x02, //     INPUT (Data,Var,Abs)
    0x95, 0x01, //     REPORT_COUNT (1)
    0x75, 0x05, //     REPORT_SIZE (5)
    0x81, 0x01, //     INPUT (Cnst,Var,Abs)

    0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30, //     USAGE (X)
    0x09, 0x31, //     USAGE (Y)
    0x09, 0x38,

    0x15, 0x81, //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F, //     LOGICAL_MAXIMUM (127)
    0x75, 0x08, //     REPORT_SIZE (8)
    0x95, 0x03, //     REPORT_COUNT (2)

    0x81, 0x06, //     INPUT (Data,Var,Rel)
    0xC0, 0x09,
    0x3c, 0x05,
    0xff, 0x09,

    0x01, 0x15,
    0x00, 0x25,
    0x01, 0x75,
    0x01, 0x95,

    0x02, 0xb1,
    0x22, 0x75,
    0x06, 0x95,
    0x01, 0xb1,

    0x01, 0xc0 //   END_COLLECTION
};

/*!< mouse report struct */
struct hid_mouse {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
};

/*!< class */
static usbd_class_t hid_class;

/*!< interface */
static usbd_interface_t hid_intf;

/*!< mouse report */
static struct hid_mouse mouse_cfg;

#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1

/*!< hid state ! Data can be sent only when state is idle  */
uint8_t hid_state = HID_STATE_IDLE;

/* function ------------------------------------------------------------------*/
static void usbd_hid_int_callback(uint8_t ep)
{
    /*!< endpoint call back */
    /*!< transfer successfully */
    if (hid_state == HID_STATE_BUSY) {
        /*!< update the state  */
        hid_state = HID_STATE_IDLE;
    }
}

/*!< endpoint call back */
static usbd_endpoint_t hid_in_ep = {
    .ep_cb = usbd_hid_int_callback,
    .ep_addr = HID_INT_EP
};

/*!< class */
usbd_class_t cdc_class;
/*!< interface one */
usbd_interface_t cdc_cmd_intf;
/*!< interface two */
usbd_interface_t cdc_data_intf;

#ifdef CONFIG_USB_HS
#define CDC_BULK_SIZE 512
#else
#define CDC_BULK_SIZE 64
#endif

/* function ------------------------------------------------------------------*/
void usbd_cdc_acm_out(uint8_t ep)
{
    uint8_t data[CDC_BULK_SIZE];
    uint32_t read_byte;

    usbd_ep_read(ep, data, CDC_BULK_SIZE, &read_byte);
    for (uint32_t i = 0; i < read_byte; i++) {
        USB_LOG_RAW("%02x ", data[i]);
    }
    USB_LOG_RAW("\r\n");
    USB_LOG_RAW("read len:%d\r\n", read_byte);
    usbd_ep_read(ep, NULL, 0, NULL);
}

void usbd_cdc_acm_in(uint8_t ep)
{
    USB_LOG_RAW("in\r\n");
}

/*!< endpoint call back */
usbd_endpoint_t cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_out
};

usbd_endpoint_t cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_in
};

/* function ------------------------------------------------------------------*/
/**
  * @brief            msc ram init
  * @pre              none
  * @param[in]        none
  * @retval           none
  */
void cdc_acm_hid_msc_descriptor_init(void)
{
    usbd_desc_register(cdc_acm_hid_msc_descriptor);

    /*!< add interface */
    usbd_cdc_add_acm_interface(&cdc_class, &cdc_cmd_intf);
    usbd_cdc_add_acm_interface(&cdc_class, &cdc_data_intf);
    /*!< interface add endpoint */
    usbd_interface_add_endpoint(&cdc_data_intf, &cdc_out_ep);
    usbd_interface_add_endpoint(&cdc_data_intf, &cdc_in_ep);

    usbd_msc_class_init(MSC_OUT_EP, MSC_IN_EP);
    /*!< add interface */
    usbd_hid_add_interface(&hid_class, &hid_intf);
    /*!< interface add endpoint */
    usbd_interface_add_endpoint(&hid_intf, &hid_in_ep);
    /*!< register report descriptor */
    usbd_hid_report_descriptor_register(1, hid_mouse_report_desc, HID_MOUSE_REPORT_DESC_SIZE);

    /*!< init mouse report data */
    mouse_cfg.buttons = 0;
    mouse_cfg.wheel = 0;
    mouse_cfg.x = 0;
    mouse_cfg.y = 0;

    usbd_initialize();
}

/**
  * @brief            hid mouse test
  * @pre              none
  * @param[in]        none
  * @retval           none
  */
void hid_mouse_test(void)
{
    /*!< move mouse pointer */
    mouse_cfg.x += 10;
    mouse_cfg.y = 0;
    usbd_ep_write(HID_INT_EP, (uint8_t *)&mouse_cfg, 4, NULL);
}

volatile uint8_t dtr_enable = 0;

void usbd_cdc_acm_set_dtr(uint8_t intf, bool dtr)
{
    if (dtr) {
        dtr_enable = 1;
    } else {
        dtr_enable = 0;
    }
}

void cdc_acm_data_send_with_dtr_test(void)
{
    if (dtr_enable) {
        uint8_t data_buffer[CDC_BULK_SIZE] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30 };
        memset(&data_buffer[10],'a',CDC_BULK_SIZE - 10);
        data_buffer[63] = 'b';
        usbd_ep_write(CDC_IN_EP, data_buffer, CDC_BULK_SIZE, NULL);// test if zlp is work.
        usbd_ep_write(CDC_IN_EP, NULL, 0, NULL);
    }
}