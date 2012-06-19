'''
Created on May 11, 2012

@author: ppc
'''

import lib.peoplepower.settings as settings
import httplib


def send(measurements):
    '''
    Send a List of Measurements
    [Measurements]
    '''
    xml = toxml(measurements)
    print xml
    
    headers = {"Content-type":"text/xml", "Content-Length":"%d" % len(xml)}
    
    connection = httplib.HTTPConnection(settings.DOMAIN, settings.UNSECURE_PORT)
    connection.request("POST", settings.DEVICEURI, "", headers)
    connection.send(xml)
    res = connection.getresponse()
        
    if 200 != res.status:
        print "SendMeasurement: HTTP Status=%s; Reason=%s" % (res.status, res.reason)
        return
        
    print res.read()
        
        
def toxml(measurements):
    '''
    Convert a List of Measurements into XML
    '''
    settings.SEQUENCE_NUM += 1
    
    xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
    xml += "<h2s ver=\"2\" proxyId=\"" + str(settings.PROXYID) + "\" seq=\"" + str(settings.SEQUENCE_NUM) + "\">"
    
    for m in measurements:
        xml += m.toxml()
            
    xml += "</h2s>\r\n"
    return xml
