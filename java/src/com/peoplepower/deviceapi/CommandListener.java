package com.peoplepower.deviceapi;

import java.util.List;

import com.peoplepower.messages.Command;

public interface CommandListener {

  /**
   * Parse a command
   * @param commands List of Commands
   */
  public void execute(List<Command> commands);
  
}
