package com.peoplepower.deviceapi;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.peoplepower.MacAddress;
import com.peoplepower.Server;
import com.peoplepower.messages.Ack;
import com.peoplepower.messages.Command;

public class SendAck {
  
  /**
   * Ack a single command
   * @param command
   */
  public static void ackSingleCommand(Command command) {
    List<Command> commands = new ArrayList<Command>();
    commands.add(command);
    ackMultipleCommands(commands);
  }
  
  /**
   * Ack a List of Commands
   * @param commands
   */
  public static void ackMultipleCommands(List<Command> commands) {
    List<Ack> acks = new ArrayList<Ack>();
    Command focusedCommand;
    
    for(Iterator<Command> it = commands.iterator(); it.hasNext(); ) {
      focusedCommand = (Command) it.next();
      acks.add(new Ack(focusedCommand.getCommandId(), DeviceToServerResultCodes.RECEIVED));
    }
    
    if(acks.size() > 0) {
      System.out.println("Acking " + acks.size() + " command(s)");
      multipleAck(acks);
      
    } else {
      System.out.println("Nothing to ack");
    }
  }

  /**
   * Send a single ack
   * @param singleAck
   */
  public static void singleAck(Ack singleAck) {
    List<Ack> acks = new ArrayList<Ack>();
    acks.add(singleAck);
    multipleAck(acks);
  }
  
  /**
   * Send multiple acks
   * @param args
   */
  public static void multipleAck(List<Ack> acks) {
    try {
      
      URL url = new URL("http://" + Server.DOMAIN + Server.DEVICEURI);
      HttpURLConnection connection = (HttpURLConnection) url.openConnection();
      connection.setDoInput(true);
      connection.setDoOutput(true);

      connection.setRequestMethod("POST");

      String content = convertToXml(acks);

      System.out.println("Outbound Ack: [" + content + "]");
      
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
        sb.append(line);
      }
      
      System.out.println("Ack Result: " + sb.toString());
      
      connection.disconnect();
      
      
    } catch (Exception e) {
      System.out.println(e);
      e.printStackTrace();
    }
    
  }
  

  /**
   * Create Device API XML from a List of Measurements containing a List of Params
   * @param measurements
   * @return xml
   */
  private static String convertToXml(List<Ack> acks) {
    String xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    xml += "<h2s ver=\"2\" proxyId=\"" + MacAddress.getMacAddress() + "\" seq=\"" + MessageSequenceNumber.nextSequenceNumber() + "\">";
    
    Ack focusedAck;
    for(Iterator<Ack> it = acks.iterator(); it.hasNext(); ) {
      focusedAck = (Ack) it.next();
      xml += "<response cmdId=\"" + focusedAck.getCommandId() + "\" result=\"" + focusedAck.getResultCode() + "\"/>";
    }
    
    xml += "</h2s>\n";
    
    return xml;
  }
}
