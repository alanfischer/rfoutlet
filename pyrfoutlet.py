"""
Python bindings for rfoutlet
"""
from ctypes import *
import os
import fnmatch
import sys
import logging

def findlib():
    dirname = os.path.dirname(__file__)
    for ext in ['so','dylib','dll']:
        names = fnmatch.filter(os.listdir(dirname),"*rfoutlet*" + ext)
        for name in names:
            try:
                return cdll.LoadLibrary(dirname +"/"+name)
            except OSError:
                pass
    return None
librfoutlet = findlib()
if librfoutlet is None:
    raise OSError("Unable to find rfoutlet library")

librfoutlet.RFOutlet_parseProduct.argtypes = [c_char_p]
librfoutlet.RFOutlet_new.argtypes = [c_int]
librfoutlet.RFOutlet_new.restype = c_void_p
librfoutlet.RFOutlet_delete.argtypes = [c_void_p]
librfoutlet.RFOutlet_sendState.argtypes = [c_void_p, c_int, c_char_p, c_int, c_bool]
librfoutlet.RFOutlet_getState.argtypes = [c_void_p, c_int, c_char_p, c_int]
c_logcb_p = CFUNCTYPE(None, c_char_p)
librfoutlet.RFOutlet_setLog.argtypes = [c_logcb_p]

if sys.version_info >= (3, 0):
    def tochar(data):
        return bytes(data,'utf-8') if data else None
else:
    def tochar(data):
        return str(data) if data else None

import logging
logger = logging.getLogger(__file__)

def logcb(data):
    logger.info(data.decode('utf-8'))

clogcb = c_logcb_p(logcb)
librfoutlet.RFOutlet_setLog(clogcb)

class RFOutlet(object):
    def __init__(self, pin):
        self.rfoutlet = librfoutlet.RFOutlet_new(pin)

    def __del__(self):
        librfoutlet.RFOutlet_delete(self.rfoutlet)

    def setState(self, product, channel, outlet, state):
        librfoutlet.RFOutlet_sendState(self.rfoutlet, product, tochar(channel), outlet, state)

    def getState(self, product, channel, outlet):
        return librfoutlet.RFOutlet_getState(self.rfoutlet, product, tochar(channel), outlet)

def parseProduct(product):
    return librfoutlet.RFOutlet_parseProduct(tochar(product))
