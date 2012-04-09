package com.peoplepower.applicationapi;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.peoplepower.MacAddress;
import com.peoplepower.Server;

public class Activate {

  /** Device Type */
  private int myDeviceType;
  
  /** Device ID */
  private String myDeviceId;
  
  /** Location ID */
  private boolean alreadyExisted = false;

  /** Response code */
  private int resultCode = -1;

  
  /**
   * Default SAX XML parser
   */
  private DefaultHandler handler = new DefaultHandler() {

    String element = "";

    public void startElement(String uri, String localName, String qName,
        Attributes attributes) throws SAXException {
      element = qName;
    }
    
    public void characters(char ch[], int start, int length)
        throws SAXException {
      if (element.equalsIgnoreCase("resultCode")) {
        resultCode = Integer.parseInt(new String(ch, start, length));
        
      } else if(element.equalsIgnoreCase("exist")) {
        alreadyExisted = (new String(ch, start, length)).equalsIgnoreCase("true");
      }
    }
  };

  
  /**
   * Activate a device to the server
   */
  public Activate(String apiKey, int locationId, String deviceId, int deviceType) {
    try {
      HttpURLConnection connection = null;
      URL serverAddress = null;
      SAXParserFactory factory = SAXParserFactory.newInstance();
      SAXParser saxParser = factory.newSAXParser();

      myDeviceId = deviceId;
      myDeviceType = deviceType;
      
      serverAddress = new URL("http://" + Server.DOMAIN + Server.APPLICATIONURI
          + "/deviceRegistration/" + apiKey + "/" + locationId + "/" + deviceId + "?deviceType=" + deviceType);

      // Set up the initial connection
      connection = (HttpURLConnection) serverAddress.openConnection();
      connection.setRequestMethod("GET");
      connection.setDoOutput(true);
      connection.setReadTimeout(10000);
      connection.connect();

      // Parse the XML using the DefaultHandler above
      saxParser.parse(connection.getInputStream(), handler);
      connection.disconnect();

      // Now the API key and result code are stored locally in this object

    } catch (MalformedURLException e) {
      e.printStackTrace();
    } catch (ProtocolException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } catch (ParserConfigurationException e) {
      e.printStackTrace();
    } catch (SAXException e) {
      e.printStackTrace();
    }

  }

  public boolean alreadyExisted() {
    return alreadyExisted;
  }

  public int getResultCode() {
    return resultCode;
  }
  
  public String getDeviceId() {
    return myDeviceId;
  }
  
  public int getDeviceType() {
    return myDeviceType;
  }

  
  public static void main(String args[]) {
    System.out.println("Logging in as example@peoplepowerco.com...");
    Login login = new Login("example@peoplepowerco.com", "password");
    
    if (login.getResultCode() == 0) {
      System.out.println("Gathering location ID for this user...");
      Location location = new Location(login.getApiKey());

      if (location.getResultCode() == 0) {
        System.out.println("Activating this computer (" + MacAddress.getMacAddress() + ") as a device on the server...");
        Activate activate = new Activate(login.getApiKey(), location.getLocationId(), MacAddress.getMacAddress(), 3);
        
        System.out.println("\nDone!");
        System.out.println("Result code = " + activate.getResultCode());
        System.out.println("Device ID = " + activate.getDeviceId());
        System.out.println("Device Type = " + activate.getDeviceType());
        
        if (activate.getResultCode() == 0) {
          if (activate.alreadyExisted()) {
            System.out.println("This device was previously activated");
          } else {
            System.out.println("This is a new device");
          }
        }
      }
    }
  }
}
