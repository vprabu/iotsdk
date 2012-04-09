package com.peoplepower.deviceapi;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.peoplepower.MacAddress;
import com.peoplepower.Server;
import com.peoplepower.messages.Command;
import com.peoplepower.messages.Measurement;
import com.peoplepower.messages.Param;

public class SendMeasurement {

  /**
   * Send a single measurement
   * @param singleMeasurement
   */
  public static List<Command> send(Measurement singleMeasurement) {
    List<Measurement> measurements = new ArrayList<Measurement>();
    measurements.add(singleMeasurement);
    return send(measurements);
  }
  
  /**
   * Send multiple measurements
   * @param args
   * @return raw command text if a command is received
   */
  public static List<Command> send(List<Measurement> measurements) {
    try {
      
      URL url = new URL("http://" + Server.DOMAIN + Server.DEVICEURI);
      HttpURLConnection connection = (HttpURLConnection) url.openConnection();
      connection.setDoInput(true);
      connection.setDoOutput(true);

      connection.setRequestMethod("POST");

      String content = convertToXml(measurements);

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
      
      connection.disconnect();
      rd.close();
      
      List<Command> commands = new CommandParser().extractCommands(sb.toString());
      SendAck.ackMultipleCommands(commands);
      
    } catch (IOException e) {
      e.printStackTrace();
    }

    return new ArrayList<Command>();
  }
  
  
  /**
   * Create Device API XML from a List of Measurements containing a List of Params
   * @param measurements
   * @return xml
   */
  private static String convertToXml(List<Measurement> measurements) {
    
    String xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    xml += "<h2s ver=\"2\" proxyId=\"" + MacAddress.getMacAddress() + "\" seq=\"" + MessageSequenceNumber.nextSequenceNumber() + "\">";
    
    Measurement focusedMeasurement;
    Param focusedParameter;
    for(Iterator<Measurement> it = measurements.iterator(); it.hasNext(); ) {
      focusedMeasurement = (Measurement) it.next();
      xml += "<measure deviceId=\"" + focusedMeasurement.getDeviceId() + "\" timestamp=\"" + focusedMeasurement.getTimestamp() + "\">";
      
      for(Iterator<Param> paramIterator = focusedMeasurement.iterator(); paramIterator.hasNext(); ) {
        focusedParameter = (Param) paramIterator.next();
        xml += "<param name=\"" + focusedParameter.getName() + "\">" + focusedParameter.getValue() + "</param>";
      }
      
      xml += "</measure>";
    }
    
    xml += "</h2s>\n";
    
    System.out.println(xml);
    return xml;
  }
}
