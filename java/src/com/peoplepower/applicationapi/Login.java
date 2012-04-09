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

public class Login {


  /** API Key */
  private String key = "";
  
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
      if(element.equalsIgnoreCase("key")) {
        key = new String(ch, start, length);
      } else if(element.equalsIgnoreCase("resultCode")) {
        resultCode = Integer.parseInt(new String(ch, start, length));
      }
    }
  };
  
  
  /**
   * Login to the server through the Application API
   * 
   * @param username
   * @param password
   */
  public Login(String username, String password) {
    try {
      HttpURLConnection connection = null;
      URL serverAddress = null;
      SAXParserFactory factory = SAXParserFactory.newInstance();
      SAXParser saxParser = factory.newSAXParser();
      
      serverAddress = new URL("http://" + Server.DOMAIN + Server.APPLICATIONURI
          + "/login/" + username + "/" + password + "/14/1");

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
  
  public String getApiKey() {
    return key;
  }

  public int getResultCode() {
    return resultCode;
  }

  public static void main(String args[]) {
    System.out.println("Logging in as example@peoplepowerco.com...");
    Login login = new Login("example@peoplepowerco.com", "password");
    System.out.println("Result = " + login.getResultCode());
    System.out.println("API Key = " + login.getApiKey());
  }
}
