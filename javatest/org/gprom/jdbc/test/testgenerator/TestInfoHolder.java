package org.gprom.jdbc.test.testgenerator;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.InvalidPropertiesFormatException;
import java.util.Vector;

import org.apache.logging.log4j.Level;
import org. gprom.jdbc.test.testgenerator.dataset.DataAndQueryGenerator;
import org.gprom.jdbc.utility.PropertyConfigurator;

public class TestInfoHolder {

	private static TestInfoHolder instance;
	
	private DataAndQueryGenerator generator;
	private String genName;
	private Vector<String> markedErrors;
	private boolean loggerConfigured = false;
	
	private TestInfoHolder () {
		markedErrors = new Vector<String> ();
	}
	
	public static TestInfoHolder getInstance () {
		if (instance == null) {
			instance = new TestInfoHolder ();
		}
		return instance;
	}
	
	public void configureLogger(String logFile) {
		if (loggerConfigured)
			return;
		PropertyConfigurator.configureHonoringProperties(logFile);
		loggerConfigured = true;
	}
	
	public DataAndQueryGenerator getCurrentGenerator () {
		return generator;
	}
	
	public void setGenerator (String name) throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		generator = new DataAndQueryGenerator (ConnectionOptions.getInstance().getPath() + "/" + name + ".xml");
		genName = name;
	}
	
	public String getGeneratorName () {
		return genName;
	}
	
	public Vector<String> getMarkedErrors () {
		return markedErrors;
	}
	
	public void resetMarkedErrors () {
		markedErrors = new Vector<String> ();
	}
	
	public void addMarkedError (String error) {
		markedErrors.add(error);
	}
	
}
