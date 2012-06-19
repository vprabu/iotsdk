'''
Created on May 12, 2012

@author: David Moss
'''

from uuid import getnode as getmac


USE_SSL = False

DEVICE_SSL_PORT = "9443"

APPLICATION_SSL_PORT = "8443"

UNSECURE_PORT = "80"

HTTPS_PREFIX = "https://"

HTTP_PREFIX = "http://"

DOMAIN = "developer.peoplepowerco.com"

APPLICATIONURI = "/espapi/rest"

DEVICEURI = "/deviceio/ml"

PROXYID = getmac()

SEQUENCE_NUM = 0