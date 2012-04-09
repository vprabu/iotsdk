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

import com.peoplepower.Server;

public class Location {

  /** Location ID */
  private int locationId = -1;

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
      
      if(element.equalsIgnoreCase("location")) {
        int length = attributes.getLength();
        
        // Find the "id" attribute for the location
        for (int i = 0; i < length; i++) {
          if(attributes.getQName(i).equalsIgnoreCase("id")) {
            locationId = Integer.parseInt(attributes.getValue(i));
          }
        }
      }
    }
    
    public void characters(char ch[], int start, int length)
        throws SAXException {
      if (element.equalsIgnoreCase("resultCode")) {
        resultCode = Integer.parseInt(new String(ch, start, length));
      }
    }
  };

  
  /**
   * Grab the Location ID for the user from the server
   * 
   * @param apiKey
   */
  public Location(String apiKey) {
    try {
      HttpURLConnection connection = null;
      URL serverAddress = null;
      SAXParserFactory factory = SAXParserFactory.newInstance();
      SAXParser saxParser = factory.newSAXParser();

      serverAddress = new URL("http://" + Server.DOMAIN + Server.APPLICATIONURI
          + "/user/" + apiKey);

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

  public int getLocationId() {
    return locationId;
  }

  public int getResultCode() {
    return resultCode;
  }

  public static void main(String args[]) {
    System.out.println("Logging in as example@peoplepowerco.com...");
    Login login = new Login("example@peoplepowerco.com", "password");
    if (login.getResultCode() == 0) {
      Location location = new Location(login.getApiKey());

      if (location.getResultCode() == 0) {
        System.out.println("API Key = " + login.getApiKey());
        System.out.println("Location ID = " + location.getLocationId());
      }
    }
  }
}
