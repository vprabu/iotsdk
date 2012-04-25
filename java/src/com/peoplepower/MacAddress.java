package com.peoplepower;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;

public class MacAddress {

  /** MAC address used when the program started */
  private static String mac = null;
  
  /** Proxy ID to override the mac address if we want */
  private static String proxyId = null;
  
  /**
   * @return the currently defined proxy ID
   */
  public static String getProxyId() {
    if(proxyId == null) {
      return getMacAddress();
    }
    
    return proxyId;
  }
  
  /**
   * Override the proxy ID with your own
   * @param id
   */
  public static void setProxyId(String id) {
    proxyId = id;
  }
  
  /**
   * Get a MAC address from this computer
   * @return MAC address
   */
  public static String getMacAddress() {
    if(mac != null) {
      return mac;
    }
    
    InetAddress ip;
    
    String mac = "0";
    
    try {
      ip = InetAddress.getLocalHost();
   
      NetworkInterface network = NetworkInterface.getByInetAddress(ip);
   
      byte[] macBytes = network.getHardwareAddress();
      
      StringBuilder sb = new StringBuilder();
      for (int i = 0; i < macBytes.length; i++) {
        sb.append(String.format("%02X", macBytes[i]));
      }
      
      mac = sb.toString();
      
    } catch (UnknownHostException e) {
      e.printStackTrace();
    } catch (SocketException e) {
      e.printStackTrace();
    }
    
    return mac;
  }
}
