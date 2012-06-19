'''
Created on May 11, 2012

@author: David Moss
'''

class Measurement(object):
    '''
    This is a class for a single Measurement
    It consists of one or more params, which are
    accessible through Measurement.param
    
    Use Measurement.param.append to add Param objects
    from the Param class
    
    Instantiate the object with the deviceId that
    generated this measurement
    '''

    params = []
    
    def __init__(self, deviceId):
        '''
        Instantiate the object with the deviceId
        of the device that captured the measurement
        '''
        self._deviceId = deviceId
        # TODO: Timestamp this measurement
        
    def toxml(self):
        '''
        Return the XML representing this measurement
        and all the params that compose it
        '''
        xml = "<measure deviceId=\"" + str(self._deviceId) + "\">"  # TODO add the timestamp
        
        for p in self.params:
            xml += "<param name=\"" + str(p.name) + "\">" + str(p.value) + "</param>"
            
        xml += "</measure>"
        
        return xml