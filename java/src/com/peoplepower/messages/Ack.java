package com.peoplepower.messages;

public class Ack {

  private int myCommandId;
  
  private int myResultCode;
  
  /**
   * Constructor
   * @param name
   * @param value
   */
  public Ack(int commandId, int resultCode) {
    myCommandId = commandId;
    myResultCode = resultCode;
  }

  public int getCommandId() {
    return myCommandId;
  }
  
  public int getResultCode() {
    return myResultCode;
  }
}
