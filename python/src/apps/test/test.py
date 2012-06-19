'''
Created on May 12, 2012

@author: David Moss
'''

import lib.peoplepower.deviceapi.sendmeasurement as send
import lib.peoplepower.messages.measurement as measurement
import lib.peoplepower.messages.alert as alert
import lib.peoplepower.messages.param as param
import lib.peoplepower.settings as settings

def main():
    print "Using proxy ID " + str(settings.PROXYID)
    m = measurement.Measurement(str(settings.PROXYID) + "-ufo")
    m.params.append(param.Param("outletStatus","ON"))
    send.send([m])
    
    #a = alert.Alert("0090f5b567f72-ufo", "LAUNDRY_DONE")
    #a.params.append(param.Param("cost","$0.85"))
    #a.params.append(param.Param("duration","60"))
    #send.send([a])
    

if __name__ == '__main__':
    main()