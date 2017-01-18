"""
Control an rf switch using librfoutlet
"""
import logging
import pyrfoutlet

from homeassistant.components.switch import PLATFORM_SCHEMA
from homeassistant.const import DEVICE_DEFAULT_NAME
from homeassistant.helpers.entity import ToggleEntity

_LOGGER = logging.getLogger(__name__)

CONF_PIN = 'pin'
CONF_OUTLETS = 'outlets'

# pylint: disable=unused-argument
def setup_platform(hass, config, add_devices, discovery_info=None):
    rfoutlet = rfoutlet.RFOutlet(config.get(CONF_PIN))
    outlets = []
    for data in config.get(CONF_OUTLETS):
        name = data['name']
        product = rfoutlet.parseProduct(data['product'])
        channel = data['channel']
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
