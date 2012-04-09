package com.peoplepower.deviceapi;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.peoplepower.messages.Command;
import com.peoplepower.messages.Param;

public class CommandParser {

  /** List of Commands received */
  private List<Command> commands;

  /**
   * Default SAX XML parser
   */
  private DefaultHandler handler = new DefaultHandler() {

    /** The current XML tag we're parsing */
    private String element = "";

    /** The focused command we're populating with Params */
    private Command focusedCommand;

    /** The focused param we're populating with a value */
    private Param focusedParam;

    public void startElement(String uri, String localName, String qName,
        Attributes attributes) throws SAXException {
      element = qName;

      if (element.equalsIgnoreCase("command")) {
        focusedCommand = new Command();
        int length = attributes.getLength();

        // Find the "id" attribute for the location
        for (int i = 0; i < length; i++) {
          if (attributes.getQName(i).equalsIgnoreCase("cmdId")) {
            focusedCommand
                .setCommandId(Integer.parseInt(attributes.getValue(i)));

          } else if (attributes.getQName(i).equalsIgnoreCase("deviceId")) {
            focusedCommand.setDeviceId(attributes.getValue(i));

          } else if (attributes.getQName(i).equalsIgnoreCase("type")) {
            focusedCommand.setType(attributes.getValue(i));
          }
        }

      } else if (element.equalsIgnoreCase("param")) {
        focusedParam = new Param();
        int length = attributes.getLength();

        // Find the "id" attribute for the location
        for (int i = 0; i < length; i++) {
          if (attributes.getQName(i).equalsIgnoreCase("name")) {
            focusedParam.setName(attributes.getValue(i));
          }
        }
      }
    }

    public void endElement(String uri, String localName, String qName)
        throws SAXException {

      if(qName.equalsIgnoreCase("param")) {
        focusedCommand.addParam(focusedParam);
      
      } else if(qName.equalsIgnoreCase("command")) {
          commands.add(focusedCommand);
      }

    }

    public void characters(char ch[], int start, int length)
        throws SAXException {
      if (element.equalsIgnoreCase("param")) {
        focusedParam.setValue(new String(ch, start, length));
      }
    }
  };

  /**
   * Extract a List of Commands from the given XML
   * 
   * @param xml
   * @return List of Commands
   */
  public List<Command> extractCommands(String xml) {
    try {
      commands = new ArrayList<Command>();

      SAXParserFactory factory = SAXParserFactory.newInstance();

      factory.newSAXParser().parse(
          new InputSource(new ByteArrayInputStream(xml.getBytes("UTF-8"))),
          handler);

    } catch (SAXException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } catch (ParserConfigurationException e) {
      e.printStackTrace();
    }

    return commands;
  }
}
