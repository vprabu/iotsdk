package com.peoplepower.messages;

public class Param {

  private String myName;
  
  private String myValue;
  
  /**
   * Empty constructor
   */
  public Param() {
  }
  
  /**
   * Constructor
   * @param name
   * @param value
   */
  public Param(String name, String value) {
    myName = name;
    myValue = value;
  }

  public void setName(String name) {
    myName = name;
  }
  
  public void setValue(String value) {
    myValue = value;
  }
  
  public String getName() {
    return myName;
  }

  public String getValue() {
    return myValue;
  }
  
}
