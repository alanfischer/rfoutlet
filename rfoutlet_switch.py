"""
Control an rf switch using librfoutlet
"""
import logging

from homeassistant.components.switch import PLATFORM_SCHEMA
from homeassistant.const import DEVICE_DEFAULT_NAME
from homeassistant.helpers.entity import ToggleEntity
from homeassistant.components import history
import homeassistant.helpers.config_validation as cv
import voluptuous as vol
import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
os.environ['LD_LIBRARY_PATH'] = os.path.dirname(os.path.abspath(__file__))

_LOGGER = logging.getLogger(__name__)

CONF_PIN315 = 'pin315'
CONF_PIN433 = 'pin433'
CONF_OUTLETS = 'outlets'

PLATFORM_SCHEMA = PLATFORM_SCHEMA.extend(
    {
        vol.Optional(CONF_PIN315): cv.positive_int,
        vol.Required(CONF_PIN433): cv.positive_int,
        vol.Required(CONF_OUTLETS): vol.All(
            cv.ensure_list,
            [
                {
                    vol.Optional('name'): cv.string,
                    vol.Optional('product'): cv.string,
                    vol.Required('channel'): cv.string,
                    vol.Optional('outlet'): cv.string,
                }
            ]
        )
    }
)

# pylint: disable=unused-argument
def setup_platform(hass, config, add_devices, discovery_info=None):
    import pyrfoutlet
    rfoutlet = pyrfoutlet.RFOutlet(config.get(CONF_PIN315), config.get(CONF_PIN433))
    outlets = []
    for data in config.get(CONF_OUTLETS):
        name = data['name']
        product = pyrfoutlet.parseProduct(data['product'])
        channel = data['channel']
        outlet = int(data['outlet'])
        outlets.append(RFOutletSwitch(rfoutlet, name, product, channel, outlet))
    add_devices(outlets)

    for outlet in outlets:
        if outlet.entity_id:
            states = history.get_last_state_changes(hass, 1, outlet.entity_id)
            if states:
                states = states[outlet.entity_id]
                if len(states) > 0:
                    state = states[0]
                    if state.state == 'on':
                        outlet.turn_on()

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
