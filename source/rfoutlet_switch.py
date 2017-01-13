"""
Control an rf switch using librfoutlet
"""
import logging
from ctypes import *

from homeassistant.components.switch import PLATFORM_SCHEMA
from homeassistant.const import DEVICE_DEFAULT_NAME
from homeassistant.helpers.entity import ToggleEntity

_LOGGER = logging.getLogger(__name__)

CONF_PIN = 'pin'
CONF_OUTLETS = 'outlets'

librfoutlet = cdll.LoadLibrary("./librfoutlet.dylib")
librfoutlet.RFOutlet_parseProduct.argtypes = [c_char_p]
librfoutlet.RFOutlet_new.argtypes = [c_int]
librfoutlet.RFOutlet_new.restype = c_void_p
librfoutlet.RFOutlet_delete.argtypes = [c_void_p]
librfoutlet.RFOutlet_sendState.argtypes = [c_void_p, c_int, c_char_p, c_int, c_bool]
librfoutlet.RFOutlet_getState.argtypes = [c_void_p, c_int, c_char_p, c_int]

class RFOutlet(object):
    def __init__(self, pin):
        self.rfoutlet = librfoutlet.RFOutlet_new(pin)

    def __del__(self):
        librfoutlet.RFOutlet_delete(self.rfoutlet)

    def setState(self, product, channel, outlet, state):
        librfoutlet.RFOutlet_sendState(self.rfoutlet, product, channel, outlet, state)

    def getState(self, product, channel, outlet):
        return librfoutlet.RFOutlet_getState(self.rfoutlet, product, channel, outlet)


# pylint: disable=unused-argument
def setup_platform(hass, config, add_devices, discovery_info=None):
    rfoutlet = RFOutlet(config.get(CONF_PIN))
    outlets = []
    for data in config.get(CONF_OUTLETS):
        name = data['name']
        product = librfoutlet.RFOutlet_parseProduct(bytes(data['product'],'utf-8'))
        channel = bytes(data['channel'],'utf-8')
        outlet = int(data['outlet'])
        outlets.append(RFOutletSwitch(rfoutlet, name, product, channel, outlet))
    add_devices(outlets)


class RFOutletSwitch(ToggleEntity):
    def __init__(self, rfoutlet, name, product, channel, outlet):
        self._name = name or DEVICE_DEFAULT_NAME
        self.rfoutlet = rfoutlet
        self.product = product
        self.channel = channel
        self.outlet = outlet

    @property
    def name(self):
        """Return the name of the switch."""
        return self._name

    @property
    def should_poll(self):
        """No polling needed."""
        return False

    @property
    def is_on(self):
        """Return true if device is on."""
        return self.rfoutlet.getState(self.product, self.channel, self.outlet)

    def turn_on(self):
        """Turn the device on."""
        self.rfoutlet.setState(self.product, self.channel, self.outlet, True)
        self.schedule_update_ha_state()

    def turn_off(self):
        """Turn the device off."""
        self.rfoutlet.setState(self.product, self.channel, self.outlet, False)
        self.schedule_update_ha_state()
