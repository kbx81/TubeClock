//
// kbx81's tube clock USB CDC-ACM serial interface
// ---------------------------------------------------------------------------
// (c)2019 by kbx81. See LICENSE for details.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//
#include <cstdint>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/usbd.h>

#include "SerialRemote.h"
#include "UsbSerial.h"


namespace kbxTubeClock {

namespace UsbSerial
{


// USB device and configuration descriptors
//
static const struct usb_device_descriptor _devDescriptor = {
  .bLength            = USB_DT_DEVICE_SIZE,
  .bDescriptorType    = USB_DT_DEVICE,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = USB_CLASS_CDC,
  .bDeviceSubClass    = 0,
  .bDeviceProtocol    = 0,
  .bMaxPacketSize0    = 64,
  .idVendor           = 0x0483,   // ST Microelectronics (development/hobby use)
  .idProduct          = 0x5740,   // Virtual COM port
  .bcdDevice          = 0x0200,
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 3,
  .bNumConfigurations = 1,
};

// Interrupt notification endpoint (required by Linux cdc_acm driver)
//
static const struct usb_endpoint_descriptor _commEndp[] = {{
  .bLength          = USB_DT_ENDPOINT_SIZE,
  .bDescriptorType  = USB_DT_ENDPOINT,
  .bEndpointAddress = 0x83,
  .bmAttributes     = USB_ENDPOINT_ATTR_INTERRUPT,
  .wMaxPacketSize   = 16,
  .bInterval        = 255,
  .extra            = nullptr,
  .extralen         = 0,
}};

// Bulk data endpoints: 0x01 = OUT (host->device), 0x82 = IN (device->host)
//
static const struct usb_endpoint_descriptor _dataEndp[] = {{
  .bLength          = USB_DT_ENDPOINT_SIZE,
  .bDescriptorType  = USB_DT_ENDPOINT,
  .bEndpointAddress = 0x01,
  .bmAttributes     = USB_ENDPOINT_ATTR_BULK,
  .wMaxPacketSize   = 64,
  .bInterval        = 1,
  .extra            = nullptr,
  .extralen         = 0,
}, {
  .bLength          = USB_DT_ENDPOINT_SIZE,
  .bDescriptorType  = USB_DT_ENDPOINT,
  .bEndpointAddress = 0x82,
  .bmAttributes     = USB_ENDPOINT_ATTR_BULK,
  .wMaxPacketSize   = 64,
  .bInterval        = 1,
  .extra            = nullptr,
  .extralen         = 0,
}};

// CDC functional descriptors
//
static const struct {
  struct usb_cdc_header_descriptor       header;
  struct usb_cdc_call_management_descriptor call_mgmt;
  struct usb_cdc_acm_descriptor          acm;
  struct usb_cdc_union_descriptor        cdc_union;
} __attribute__((packed)) _cdcFunctional = {
  .header = {
    .bFunctionLength    = sizeof(struct usb_cdc_header_descriptor),
    .bDescriptorType    = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
    .bcdCDC             = 0x0110,
  },
  .call_mgmt = {
    .bFunctionLength    = sizeof(struct usb_cdc_call_management_descriptor),
    .bDescriptorType    = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
    .bmCapabilities     = 0,
    .bDataInterface     = 1,
  },
  .acm = {
    .bFunctionLength    = sizeof(struct usb_cdc_acm_descriptor),
    .bDescriptorType    = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_ACM,
    .bmCapabilities     = 0,
  },
  .cdc_union = {
    .bFunctionLength        = sizeof(struct usb_cdc_union_descriptor),
    .bDescriptorType        = CS_INTERFACE,
    .bDescriptorSubtype     = USB_CDC_TYPE_UNION,
    .bControlInterface      = 0,
    .bSubordinateInterface0 = 1,
  },
};

// Communication interface (CDC control)
//
static const struct usb_interface_descriptor _commIface[] = {{
  .bLength            = USB_DT_INTERFACE_SIZE,
  .bDescriptorType    = USB_DT_INTERFACE,
  .bInterfaceNumber   = 0,
  .bAlternateSetting  = 0,
  .bNumEndpoints      = 1,
  .bInterfaceClass    = USB_CLASS_CDC,
  .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
  .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
  .iInterface         = 0,
  .endpoint           = _commEndp,
  .extra              = &_cdcFunctional,
  .extralen           = sizeof(_cdcFunctional),
}};

// Data interface (bulk data transfer)
//
static const struct usb_interface_descriptor _dataIface[] = {{
  .bLength            = USB_DT_INTERFACE_SIZE,
  .bDescriptorType    = USB_DT_INTERFACE,
  .bInterfaceNumber   = 1,
  .bAlternateSetting  = 0,
  .bNumEndpoints      = 2,
  .bInterfaceClass    = USB_CLASS_DATA,
  .bInterfaceSubClass = 0,
  .bInterfaceProtocol = 0,
  .iInterface         = 0,
  .endpoint           = _dataEndp,
  .extra              = nullptr,
  .extralen           = 0,
}};

static const struct usb_interface _ifaces[] = {{
  .cur_altsetting = nullptr,
  .num_altsetting = 1,
  .iface_assoc    = nullptr,
  .altsetting     = _commIface,
}, {
  .cur_altsetting = nullptr,
  .num_altsetting = 1,
  .iface_assoc    = nullptr,
  .altsetting     = _dataIface,
}};

static const struct usb_config_descriptor _cfgDescriptor = {
  .bLength              = USB_DT_CONFIGURATION_SIZE,
  .bDescriptorType      = USB_DT_CONFIGURATION,
  .wTotalLength         = 0,
  .bNumInterfaces       = 2,
  .bConfigurationValue  = 1,
  .iConfiguration       = 0,
  .bmAttributes         = 0x80,   // Bus-powered
  .bMaxPower            = 0x32,   // 100 mA
  .interface            = _ifaces,
};

static const char* _usbStrings[] = {
  "kbx81",
  "TubeClock",
  "000001",
};

// Control request buffer (EP0 data stage)
//
static uint8_t _controlBuffer[64];

// USB device handle
//
static usbd_device* _usbDev = nullptr;

// Host connection state (true when host has asserted DTR)
//
static volatile bool _connected = false;


// --- Callbacks ---

static enum usbd_request_return_codes _controlRequest(usbd_device* usbd_dev,
  struct usb_setup_data* req, uint8_t** buf, uint16_t* len,
  usbd_control_complete_callback* complete)
{
  (void)usbd_dev;
  (void)buf;
  (void)complete;

  switch (req->bRequest)
  {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
      // DTR is bit 0 of wValue
      _connected = (req->wValue & 0x0001) != 0;
      return USBD_REQ_HANDLED;

    case USB_CDC_REQ_SET_LINE_CODING:
      if (*len < sizeof(struct usb_cdc_line_coding))
        return USBD_REQ_NOTSUPP;
      return USBD_REQ_HANDLED;

    default:
      return USBD_REQ_NOTSUPP;
  }
}

// Bulk OUT callback: data received from host -> feed into SerialRemote
//
static void _dataRxCb(usbd_device* usbd_dev, uint8_t ep)
{
  (void)ep;

  char buf[64];
  uint16_t len = usbd_ep_read_packet(usbd_dev, 0x01, buf, sizeof(buf));

  for (uint16_t i = 0; i < len; i++)
  {
    SerialRemote::rxByte(buf[i]);
  }
}

static void _setConfig(usbd_device* usbd_dev, uint16_t wValue)
{
  (void)wValue;

  usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK,      64, _dataRxCb);
  usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK,      64, nullptr);
  usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, nullptr);

  usbd_register_control_callback(usbd_dev,
    USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
    USB_REQ_TYPE_TYPE  | USB_REQ_TYPE_RECIPIENT,
    _controlRequest);
}


// --- Public interface ---

void initialize()
{
  rcc_periph_clock_enable(RCC_USB);

  // PA11 (D-) and PA12 (D+): set to analog mode so GPIO doesn't interfere
  gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO11 | GPIO12);

  _usbDev = usbd_init(&st_usbfs_v2_usb_driver,
                      &_devDescriptor,
                      &_cfgDescriptor,
                      _usbStrings, 3,
                      _controlBuffer, sizeof(_controlBuffer));

  usbd_register_set_config_callback(_usbDev, _setConfig);

  // USB ISR handles enumeration and transfer events responsively.
  // poll() is also called from Application::loop() for proactive TX; it uses
  // nvic_disable_irq to prevent re-entrancy when called from thread context.
  nvic_set_priority(NVIC_USB_IRQ, 64);
  nvic_enable_irq(NVIC_USB_IRQ);
}


void poll()
{
  if (_usbDev == nullptr) return;
  // Disable USB IRQ so this can be called safely from both the USB ISR and the
  // main loop without re-entrancy.  When called from usb_isr() this is a no-op
  // (same-priority interrupts can't nest on Cortex-M0).
  nvic_disable_irq(NVIC_USB_IRQ);
  usbd_poll(_usbDev);
  nvic_enable_irq(NVIC_USB_IRQ);
}


void write(const uint8_t* data, uint16_t len)
{
  if (!_connected || _usbDev == nullptr || len == 0 || len > 64) return;
  // Disable USB IRQ so the endpoint write is not racing with usbd_poll() in
  // the ISR.
  nvic_disable_irq(NVIC_USB_IRQ);
  usbd_ep_write_packet(_usbDev, 0x82, data, len);
  nvic_enable_irq(NVIC_USB_IRQ);
}


bool isConnected()
{
  return _connected;
}


}

}
