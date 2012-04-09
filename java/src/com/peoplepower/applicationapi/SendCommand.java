package com.peoplepower.applicationapi;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.peoplepower.MacAddress;
import com.peoplepower.Server;
import com.peoplepower.deviceapi.MessageSequenceNumber;
import com.peoplepower.messages.Command;
import com.peoplepower.messages.Param;

public class SendCommand {

  /**
   * Send a single Command
   * @param singleCommand
   */
  public static String send(String apiKey, int locationId, Command singleCommand) {
    List<Command> Commands = new ArrayList<Command>();
    Commands.add(singleCommand);
    return send(apiKey, locationId, Commands);
  }
  
  /**
   * Send multiple Commands
   * @param args
   * @return raw command text if a command is received
   */
  public static String send(String apiKey, int locationId, List<Command> Commands) {
    try {
      
      URL url = new URL("http://" + Server.DOMAIN + Server.APPLICATIONURI + "/deviceParameters/" + apiKey + "/" + locationId);
      HttpURLConnection connection = (HttpURLConnection) url.openConnection();
      connection.setDoInput(true);
      connection.setDoOutput(true);

      connection.setRequestMethod("POST");

      String content = convertToXml(Commands);

      connection.setRequestProperty("Content-length", String.valueOf(content.length()));
      connection.setRequestProperty("Content-Type", "application/xml");
      connection.setRequestProperty("User-Agent", "Java");

      // open up the output stream of the connection
      DataOutputStream output = new DataOutputStream(connection.getOutputStream());

      // write out the data
      output.writeBytes(content);
      output.close();

      // get ready to read the response
      BufferedReader rd = new BufferedReader(new InputStreamReader(
          connection.getInputStream()));

      StringBuilder sb = new StringBuilder();
      String line;

      while ((line = rd.readLine()) != null) {
        sb.append(line + '\n');
      }
      
      connection.disconnect();
      rd.close();
      
      return sb.toString();
      
    } catch (IOException e) {
      e.printStackTrace();
    }

    return "";
  }
  
  
  /**
   * Create command XML from a List of Commands
   * @param Commands
   * @return xml
   */
  private static String convertToXml(List<Command> commands) {
    
    String xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    xml += "<request><devices>";
    
    Command focusedCommand;
    Param focusedParam;
    for(Iterator<Command> commandIterator = commands.iterator(); commandIterator.hasNext(); ) {
      focusedCommand = (Command) commandIterator.next();
      
      xml += "<device id=\"" + focusedCommand.getDeviceId() + "\">";
      xml += "<parameters>";
      
      for(Iterator<Param> paramIterator = focusedCommand.iterator(); paramIterator.hasNext(); ) {
        focusedParam = (Param) paramIterator.next();
        xml += "<param name=\"" + focusedParam.getName() + "\">" + focusedParam.getValue() + "</param>";
      }
      
      xml += "</parameters>";
      xml += "</device>";
    }
    
    xml += "</devices></request>";
    
    System.out.println(xml);
    return xml;
  }
  
  
  public static void main(String args[]) {
    System.out.println("Logging in as example@peoplepowerco.com...");
    
    // 1. Login
    Login login = new Login("example@peoplepowerco.com", "password");
    if (login.getResultCode() != 0) {
      System.err.println("Error! Got resultCode " + login.getResultCode());
      System.exit(1);
    }
    
    // 2. Get your location ID
    System.out.println("Gathering location ID for this user...");
    Location location = new Location(login.getApiKey());
    
    if (location.getResultCode() != 0) {
      System.err.println("Error! Got resultCode " + location.getResultCode());
      System.exit(1);
    }
    
    // 3. Activating this device (this could normally be done through your UI)
    System.out.println("Activating this computer (" + MacAddress.getMacAddress() + ") as a device on the server...");
    Activate activate = new Activate(login.getApiKey(), location.getLocationId(), MacAddress.getMacAddress(), 3);
     
    if(activate.getResultCode() != 0) {
      System.err.println("Error! Got resultCode " + activate.getResultCode());
      System.exit(1);
    }
    
    
    String output = SendCommand.send(login.getApiKey(), location.getLocationId(), new Command(MacAddress.getMacAddress(), new Param("outletStatus", "ON")));
    System.out.println(output);
  }
}
