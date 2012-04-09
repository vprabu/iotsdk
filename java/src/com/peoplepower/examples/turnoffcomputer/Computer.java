package com.peoplepower.examples.turnoffcomputer;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;

import com.peoplepower.MacAddress;
import com.peoplepower.applicationapi.Activate;
import com.peoplepower.applicationapi.Location;
import com.peoplepower.applicationapi.Login;
import com.peoplepower.deviceapi.CommandListener;
import com.peoplepower.deviceapi.DeviceToServerResultCodes;
import com.peoplepower.deviceapi.SendAck;
import com.peoplepower.deviceapi.SendMeasurement;
import com.peoplepower.deviceapi.ReceiveInstantCommands;
import com.peoplepower.messages.Ack;
import com.peoplepower.messages.Command;
import com.peoplepower.messages.Measurement;
import com.peoplepower.messages.Param;

public class Computer implements CommandListener {

  /** 
   * Constructor
   * 
   * Open a persisten connection
   * 
   * Send a measurement declaring the computer is on
   */
  public Computer() {
    // 1. Open a persistent connection with the server to receive 
    //    commands instantly.
    ReceiveInstantCommands commands = new ReceiveInstantCommands(MacAddress.getMacAddress());
    commands.addListener(this);
    commands.start();
    
    // 2. Send a measurement saying we're on
    Measurement myMeasurement = new Measurement(MacAddress.getMacAddress());
    myMeasurement.addParam(new Param("outletStatus", "ON"));
    execute(SendMeasurement.send(myMeasurement));
  }
  

  /**
   * Interpret and execute commands
   */
  public void execute(List<Command> commands) {
    Command focusedCommand;
    Param focusedParam;
    
    for(Iterator<Command> commandIterator = commands.iterator(); commandIterator.hasNext(); ) {
      focusedCommand = (Command) commandIterator.next();
      
      if(focusedCommand.getDeviceId().equalsIgnoreCase(MacAddress.getMacAddress())) {
        // This command is for me. Ack it SUCCESS because we'll execute it.
        SendAck.singleAck(new Ack(focusedCommand.getCommandId(), DeviceToServerResultCodes.SUCCESS));
        
        for(Iterator<Param> paramIterator = focusedCommand.iterator(); paramIterator.hasNext(); ) {
          focusedParam = (Param) paramIterator.next();
          
          if(focusedParam.getName().equalsIgnoreCase("outletStatus")) {
            
            if(focusedParam.getValue().equalsIgnoreCase("OFF")) {
              // Send a measurement saying the computer is off
              // Then turn the computer off
              Measurement myMeasurement = new Measurement(MacAddress.getMacAddress());
              myMeasurement.addParam(new Param("outletStatus", "OFF"));
              execute(SendMeasurement.send(myMeasurement));
              shutdown();  
            }
          }
        }
      }
    }
  }

  /**
   * Method to shutdown the OS
   */
  private void shutdown() {
    String shutdownCommand;
    String operatingSystem = System.getProperty("os.name");

    if ("Linux".equals(operatingSystem) || "Mac OS X".equals(operatingSystem)) {
        shutdownCommand = "sudo shutdown -h now";
        
    } else if ("Windows".equals(operatingSystem)) {
        shutdownCommand = "shutdown.exe -s -t 0";
        
    } else {
        throw new RuntimeException("Unsupported operating system.");
    }

    try {
      System.out.println("Shutdown command: " + shutdownCommand);
      Runtime.getRuntime().exec(shutdownCommand);
    } catch (IOException e) {
      e.printStackTrace();
    }
    
    System.exit(0);
  }
  
  /**
   * @param args
   */
  public static void main(String[] args) {
    new Computer();
  }
  
}
