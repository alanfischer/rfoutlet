"""
Python bindings for rfoutlet
"""
from ctypes import *
import os

librfoutlet = cdll.LoadLibrary(os.path.dirname(__file__) + "/librfoutlet.dylib")
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
        librfoutlet.RFOutlet_sendState(self.rfoutlet, product, bytes(channel,'utf-8'), outlet, state)

    def getState(self, product, channel, outlet):
        return librfoutlet.RFOutlet_getState(self.rfoutlet, product, bytes(channel,'utf-8'), outlet)

def parseProduct(product):
    return librfoutlet.RFOutlet_parseProduct(bytes(product,'utf-8'));
