/**
 * 
 */
package org.gprom.jdbc.test.testgenerator;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.InvalidPropertiesFormatException;
import java.util.Properties;
import java.util.Vector;

import org.gprom.jdbc.driver.GProMConnection;


/**
 *
 * Part of Project PermTester
 * @author Boris Glavic
 *
 */
public class OptionsManager {

	static OptionsManager instance;
	
	private Vector<String> currentSettings;
	private Properties givenSettings;
	private Vector<String> options;
	
	private OptionsManager () throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		options = new Vector<String> ();
		currentSettings = new Vector<String> ();
		
		givenSettings = new Properties ();
		givenSettings.loadFromXML(new FileInputStream(ConnectionOptions.getInstance().getPath() + "/settings.xml"));
					
		createInitialSettings ();
	}
	
	public static OptionsManager getInstance () throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		if (instance == null) {
			instance = new OptionsManager ();
		}
		return instance;
	}
	
	public void reloadOptions() throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		options = new Vector<String> ();
		currentSettings = new Vector<String> ();
		
		givenSettings = new Properties ();
		givenSettings.loadFromXML(new FileInputStream(ConnectionOptions.getInstance().getPath() + "/settings.xml"));
					
		createInitialSettings ();
	}
	
	public void resetOptions () throws Exception {
		setOptions();
	}
	
	public void resetOptions (Connection con) throws Exception {
		setOptions((GProMConnection) con);
	}
	
	public int getNumOptions () {
		return options.size();
	}
	
	public int getNumSettings () {
		int result = 0;
		
		while(givenSettings.getProperty("setting." + (result + 1) + ".1") != null)
		{
			result ++;
		}
		
		return result;
	}
	
	public void setOptions (String[] values) throws Exception {
		if (values.length != options.size()) {
			throw new Exception ("wrong number of options");
		}
		
		for (int i = 0; i < values.length; i++) {
			currentSettings.set(i, values[i]);
		}
		
		setOptions ();
	}
	
	public void setOptions (int settingId) throws Exception {
		String currentKey;
		
		if (givenSettings.getProperty("setting." + settingId + ".1") == null) {
			throw new Exception ("setting " + settingId + " does not exist.");
		}
		
		for (int i = 0; i < options.size(); i++) {
			currentKey = "setting." + settingId + "." + (i + 1);
			currentSettings.set(i, givenSettings.getProperty(currentKey));
		}
		
		setOptions ();
	}
	
	public void setDefaultOptions () throws Exception {
		for (int i = 0; i < options.size(); i++) {
			currentSettings.set(i, givenSettings.getProperty("default." + (i + 1)));
		}
		
		setOptions();
	}
	
	private void createInitialSettings () {
		int curSetPos;
		String curSetName;
		
		curSetPos = 1;
		
		while ((curSetName = givenSettings.getProperty("option." + curSetPos)) != null) {
			options.add(curSetName);
			currentSettings.add("false");
			curSetPos++;
		}
	}
	
	private void setOptions (GProMConnection con) throws SQLException {	
		for (int i = 0; i < options.size(); i++) {
			con.setProperty(options.get(i), currentSettings.get(i));
		}
	}
	
	private void setOptions () throws Exception {
		setOptions (ConnectionManager.getInstance().getGProMConnection());
	}
	
}
