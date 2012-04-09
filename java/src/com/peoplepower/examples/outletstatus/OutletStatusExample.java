package com.peoplepower.examples.outletstatus;

import java.util.List;

import com.peoplepower.MacAddress;
import com.peoplepower.applicationapi.Activate;
import com.peoplepower.applicationapi.Location;
import com.peoplepower.applicationapi.Login;
import com.peoplepower.deviceapi.CommandListener;
import com.peoplepower.deviceapi.SendMeasurement;
import com.peoplepower.deviceapi.ReceiveInstantCommands;
import com.peoplepower.messages.Command;
import com.peoplepower.messages.Measurement;
import com.peoplepower.messages.Param;

public class OutletStatusExample implements CommandListener {

  public OutletStatusExample() {
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
    
    ReceiveInstantCommands commands = new ReceiveInstantCommands(MacAddress.getMacAddress());
    commands.start();
    
    // 4. Capture a measurement
    Measurement myMeasurement = new Measurement(MacAddress.getMacAddress());
    myMeasurement.addParam(new Param("outletStatus", "ON"));
    
    // 5. Send the measurement
    execute(SendMeasurement.send(myMeasurement));
    
  }
  
  /**
   * @param args
   */
  public static void main(String[] args) {
    new OutletStatusExample();
  }

  
  public void execute(List<Command> commands) {
    if(commands.size() > 0) {
      System.out.println("Received " + commands.size() + " command(s)");
    }
  }

}
