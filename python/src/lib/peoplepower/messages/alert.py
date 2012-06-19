'''
Created on May 11, 2012

@author: David Moss
'''

class Alert(object):
    '''
    This is a class for a single Alert
    It consists of zero or more params, which are
    accessible through Alert.param
    
    Use Alert.param.append to add Param objects
    from the Param class
    
    Instantiate the object with the deviceId that
    generated the and the alert type like "MOTION_DETECTED"
    '''
    
    def __init__(self, deviceId, type):
        '''
        Instantiate the object with the deviceId
        of the device that captured the measurement, and
        the type of alert. Consult with People Power about
        supporting your alert format.
        '''
        self._deviceId = deviceId
        self._type = type
        self.params = []
        # TODO: Timestamp this alert
        
    def toxml(self):
        '''
        Return the XML representing this measurement
        and all the params that compose it
        '''
        xml = "<alert deviceId=\"" + str(self._deviceId) + "\" type=\"" + str(self._type) + "\">"  # TODO add the timestamp
        
        for p in self.params:
            xml += "<param name=\"" + str(p.name) + "\">" + str(p.value) + "</param>"
            
        xml += "</alert>"
        
        return xml
    
