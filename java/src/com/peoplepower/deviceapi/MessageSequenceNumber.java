package com.peoplepower.deviceapi;

public class MessageSequenceNumber {

  /**
   * Sequence number
   */
  private static int sequence;
  
  /**
   * @return the next sequence number
   */
  protected static int nextSequenceNumber() {
    sequence++;
    return sequence;
  }
}
