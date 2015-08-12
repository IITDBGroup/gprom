package org.gprom.jdbc.driver;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class checkJDBC {
public static boolean checkJDBC()
{
	try {
	    BufferedReader in = new BufferedReader(new FileReader("java.txt"));
	    String str;
	    while ((str = in.readLine()) != null)
	       if(str.equals("on")){
	    	   return true;
	       }
	       else{
	    	   return false;
	       }
	    in.close();
	} catch (IOException e) {
	}
	return false;
}
}
