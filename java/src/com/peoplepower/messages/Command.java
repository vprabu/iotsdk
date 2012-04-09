package com.peoplepower.messages;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class Command {

  /** The List of params composing this command */
  private List<Param> myParams;
  
  /** The device that generated this command */
  private String myDeviceId;
  
  /** Command type, when receiving a command at the device */
  private String myType;
  
  /** Command id, when receiving a command at the device */
  private int myCommandId;
  
  /**
   * Empty constructor
   */
  public Command() {
    myParams = new ArrayList<Param>();
  }
  
  
  /**
   * Constructor
   */
  public Command(String deviceId) {
    myDeviceId = deviceId;
    myParams = new ArrayList<Param>();
  }
  
  /**
   * Constructor
   */
  public Command(String deviceId, Param param) {
    myDeviceId = deviceId;
    myParams = new ArrayList<Param>();
    myParams.add(param);
  }
  
  /**
   * Constructor, typically used when receiving a command
   * @param deviceId
   * @param commandId
   * @param type
   */
  public Command(String deviceId, int commandId, String type) {
    myDeviceId = deviceId;
    myCommandId = commandId;
    myType = type;
  }
  
  /**
   * Set the command ID when we receive a command at the device
   * @param commandId
   */
  public void setCommandId(int commandId) {
    myCommandId = commandId;
  }
  
  public int getCommandId() {
    return myCommandId;
  }
  
  /**
   * Set the command type when we receive a command at the device
   * @param type
   */
  public void setType(String type) {
    myType = type;
  }
  
  public String getType() {
    return myType;
  }
  
  public void addParam(Param param) {
    myParams.add(param);
  }
  
  public Iterator<Param> iterator() {
    return myParams.iterator();
  }
  
  public void setDeviceId(String deviceId) {
    myDeviceId = deviceId;
  }
  
  public String getDeviceId() {
    return myDeviceId;
  }
  
}
