'''
Created on May 11, 2012

@author: David Moss
'''

class Param(object):
    '''
    Measurements are composed of multiple params
    
    This is a single parameter (data point) to pass
    to the server inside of a measurement, possibly
    with other data points from the measurement.
    
    Use Param.name, Param.value, and Param.index to
    modify this object's state
    '''
    
    def __init__(self, name=None, value=None, index=None):
        '''
        The name of the param must be a valid recognized
        name on the People Power cloud
        
        The value can be anything you want
        
        The index can be left None or filled in if you
        have a parameter with multiple indices (i.e.
        if you have a 2 socket powerstrip, one socket
        will be index="0" and the other socket will be
        index="1")
        '''
        self.name = name
        self.value = value
        self.index = index