package com.peoplepower;


public class Server {

  /** SSL is not yet implemented in the Java IOTSDK */
  public static final boolean USE_SSL = false;
  
  public static final String DEVICE_SSL_PORT = "9443";
  
  public static final String APPLICATION_SSL_PORT = "8443";
  
  public static final String UNSECURE_PORT = "80";
  
  public static final String HTTPS_PREFIX = "https://";
  
  public static final String HTTP_PREFIX = "http://";
  
	public static final String DOMAIN = "developer.peoplepowerco.com";
	
	public static final String APPLICATIONURI = "/espapi/rest";
	
	public static final String DEVICEURI = "/deviceio/ml";
	
	public static String getPrefix() {
	  if(USE_SSL) {
	    return HTTPS_PREFIX;
	  } else {
	    return HTTP_PREFIX;
	  }
	}
}
