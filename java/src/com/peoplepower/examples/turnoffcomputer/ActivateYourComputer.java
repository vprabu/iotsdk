package com.peoplepower.examples.turnoffcomputer;

import com.peoplepower.MacAddress;
import com.peoplepower.applicationapi.Activate;
import com.peoplepower.applicationapi.Location;
import com.peoplepower.applicationapi.Login;

public class ActivateYourComputer {

  /**
   * Run this program once to activate your computer as a device on the server
   * 
   * This will use the Application API to log in and activate
   * your computer as a device. When this code is finished
   * executing, it means your computer will be connected to the
   * user account below to be controlled remotely.
   */
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
    
    System.out.println("Done! Your computer with device ID " + MacAddress.getMacAddress() + " has been linked to your account");
  }
  

}
