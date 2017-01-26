RFOutlet library
------------------

A Raspberry PI C++ library with python bindings for controlling Holiday Time RF outlets.

It needs a 315 MHz transmitter module hooked up to a GPIO pin.

The outlets are specified with a board revision, channel, and outlet number.

If unsure of the board revision, try REV2 and REV3.

Dependencies
------------
    - CMake 2.8 or Python 2 or 3

How to use
----------
    - Executable command line interface for sending one shot on/off commands
        .. code-block::

            ./rfoutlet [gpio-pin] [product-revision] [channel] [number] [on/off]

    - Python object
        .. code-block:: python

            import pyrfoutlet
            import logging

            logging.basicConfig(level=logging.INFO) # Get log messages from pyrfoutlet

            pin = 4
            outlet = pyrfoutlet.RFOutlet(pin)

            product = pyrfoutlet.parseProduct("REV3")
            channel = "F"
            outlet = 1
            outlet.setState(product,channel,outlet,True)

License
-------
    - MIT License

Any questions please contact: alan@lightningtoads.com
