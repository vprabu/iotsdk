package com.peoplepower.deviceapi;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.peoplepower.MacAddress;
import com.peoplepower.Server;
import com.peoplepower.messages.Command;

public class ReceiveInstantCommands extends Thread {

  /** The device ID observing the persistent connection */
  private String myDeviceId;

  /** True if this thread is running */
  private boolean running = false;

  /** HTTP Connection */
  private HttpURLConnection connection;

  /** Timeout in seconds */
  private static final int TIMEOUT = 5;

  /** List of command listeners */
  private Set<CommandListener> listeners;

  /**
   * Constructor
   * 
   * @param deviceId
   */
  public ReceiveInstantCommands(String deviceId) {
    myDeviceId = deviceId;
    listeners = Collections.synchronizedSet(new HashSet<CommandListener>());
  }

  public void addListener(CommandListener l) {
    listeners.add(l);
  }

  public void removeListener(CommandListener l) {
    listeners.remove(l);
  }

  /**
   * Close down the connection
   */
  public void close() {
    running = false;
    if (connection != null) {
      connection.disconnect();
    }
  }

  public void run() {
    running = true;

    while (running) {
      try {
        HttpURLConnection connection = null;
        URL serverAddress = null;

        serverAddress = new URL("http://" + Server.DOMAIN + Server.DEVICEURI
            + "?id=" + myDeviceId + "&timeout=" + TIMEOUT);

        // Set up the initial connection
        connection = (HttpURLConnection) serverAddress.openConnection();
        connection.setRequestMethod("GET");
        // connection.setDoOutput(false);
        // connection.setDoInput(true);
        connection.setReadTimeout(TIMEOUT * 1000);
        connection.setConnectTimeout(TIMEOUT * 1000);
        connection.connect();

        // read the result from the server
        BufferedReader rd = new BufferedReader(new InputStreamReader(
            serverAddress.openStream()));

        StringBuilder sb = new StringBuilder();
        String line;

        while ((line = rd.readLine()) != null) {
          sb.append(line);
        }

        line = sb.toString();
        
        if(line.length() > 1) {
          System.out.println("Received: [" + line + "]");
          List<Command> receivedCommands = new CommandParser().extractCommands(line);
          SendAck.ackMultipleCommands(receivedCommands);
          
          if(receivedCommands.size() > 0) {
            for (Iterator<CommandListener> it = listeners.iterator(); it.hasNext();) {
              ((CommandListener) it.next()).execute(receivedCommands);
            } 
          }
        }
        
        connection.disconnect();
        rd.close();
        

      } catch (MalformedURLException e) {
        e.printStackTrace();
      } catch (ProtocolException e) {
        e.printStackTrace();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  public static void main(String args[]) {
    (new ReceiveInstantCommands(MacAddress.getMacAddress())).start();
  }

}
