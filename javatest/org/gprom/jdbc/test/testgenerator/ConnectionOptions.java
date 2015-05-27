/**
 * 
 */
package org.gprom.jdbc.test.testgenerator;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

/**
 *
 * Part of Project PermTester
 * @author Boris Glavic
 *
 */
public class ConnectionOptions {


	private static ConnectionOptions instance;
	private Properties props = new Properties();
	
	private ConnectionOptions () throws IOException {
//		init();
	}
	
	public void init () throws IOException {
		File pFile = new File(System.getProperty("generator.resourcedir") + "/options.txt");
		if (!pFile.exists())
			throw new FileNotFoundException(pFile.toString());
		props = new Properties ();
		
		props.load(new FileInputStream(pFile));		
	}
	
	public static ConnectionOptions getInstance () throws FileNotFoundException, IOException {
		if (instance == null) 
			instance = new ConnectionOptions ();
		
		return instance;
	}
	
	public String getBackend() {
		return props.getProperty("Backend");
	}
	
	public String getBackendOption (String key) {
		return props.getProperty(getBackend() + "." + key);
	}
	
	public String getHost () {
		return props.getProperty("Host");
	}
	
	public int getPort () {
		return Integer.valueOf(props.getProperty("Port"));
	}
	
	
	
	public String getDbName () {
		return props.getProperty("DBName");
	}
	
	public String getUser () {
		System.out.println(props.get("User"));
		return props.getProperty("User");
	}
	
	public String getPassword () {
		return props.getProperty("Password");
	}
	
	public String getSchema () {
		return props.getProperty("Schema");
	}
	
	public String getPath () {
		return props.getProperty("Path");
	}
	
	public void setPath (String path) {
		File filePath = new File (path);
//		Properties newProp = new Properties();
		
		try {
			props.clear();
			props.load(new FileInputStream(new File(filePath, "options.txt")));
			props.setProperty("Path", path);
//			props.setProperty("User", newProp.getProperty("User"));
//			props.setProperty("Password", newProp.getProperty("Password"));
//			props.setProperty("DBName", newProp.getProperty("DBName"));
//			props.setProperty("Port", newProp.getProperty("Port"));
//			props.setProperty("Host", newProp.getProperty("Host"));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
