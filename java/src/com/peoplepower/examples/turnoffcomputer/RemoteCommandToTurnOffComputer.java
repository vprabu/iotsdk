package com.peoplepower.examples.turnoffcomputer;

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
import com.peoplepower.applicationapi.Location;
import com.peoplepower.applicationapi.Login;
import com.peoplepower.applicationapi.SendCommand;
import com.peoplepower.deviceapi.MessageSequenceNumber;
import com.peoplepower.messages.Command;
import com.peoplepower.messages.Param;

public class RemoteCommandToTurnOffComputer {
  
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
    
    
    String output = SendCommand.send(login.getApiKey(), location.getLocationId(), new Command(MacAddress.getMacAddress(), new Param("outletStatus", "OFF")));
    System.out.println(output);
  }
}
