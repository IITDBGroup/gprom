/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.InvalidPropertiesFormatException;
import java.util.List;
import java.util.Properties;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.dbunit.dataset.DataSetException;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.xml.XmlDataSet;


/**
 *
 * Part of Project GProM
 * @author Boris Glavic
 *
 */
public class DataAndQueryGenerator {

	static Logger log = Logger.getLogger(DataAndQueryGenerator.class.getName());
	
	private static final String header = "<!DOCTYPE dataset SYSTEM \"dataset.dtd\">\n" +
			"<dataset>\n" +
			"\t<table name=\"transformed\">\n";
	private static final String footer = "\t</table>\n" +
			"</dataset>";
	
	private Properties properties;
	private int numTests;
	
	public DataAndQueryGenerator (String file) throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		properties = new Properties ();
		properties.loadFromXML(new FileInputStream(file));
		numTests = calculateNumTests();
	}
	
	public DataAndQueryGenerator (Properties properties) {
		this.properties = properties;
	}
	
	public Properties getOptions () {
		String props = properties.getProperty("options");
		String[] lines;
		Properties result;
		
		if (props == null)
			return null;
		
		if(props == null || props.equals("")) {
			return null;
		}
		
		result = new Properties();		
		props = props.replaceAll("\\n", "");
		
		lines = props.split(",");  
		
		for (int i = 0; i < lines.length; i++) {
			result.setProperty(lines[i].split("=")[0], lines[i].split("=")[1]);
		}
		
		return result;
	}
	
	public DBTable getExpectedResult (String key) throws DataSetException, IOException {
		DBTable result;
		
		return  getResult (key + ".result");
	}
	
	public DBTable[] getExpectedResults (String key) throws DataSetException {
		Vector<DBTable> expectedResults;
		DBTable[] result;
		String resultKey;
		int num;
		
		expectedResults = new Vector<DBTable> ();
		
		expectedResults.add (getResult(key + ".result"));
		
		num = 1;
		resultKey = key + ".result" + num;
		while((properties.getProperty(resultKey)) != null) {
			expectedResults.add (getResult(resultKey));
			num++;
			resultKey = key + ".result" + num;
		}
		
		result = expectedResults.toArray(new DBTable[0]);
		
		return result;
	}
	
	public boolean isInExcludes (int settingsNum, int qNum) {
		int[] excludes;
		
		excludes = getExcludes(qNum);
		if (excludes == null)
			return false;
		
		for (int i = 0; i < excludes.length; i++) {
			if (excludes[i] == settingsNum)
				return true;
		}
		
		return false;
	}
	
	public int[] getExcludes (int qNum) {
		return getExcludes ("q" + qNum);
	}
	
	public int[] getExcludes (String key) {
		String excludes;
		String[] excludeSplit;
		int[] excludeSettings;
		
		excludes = properties.getProperty(key + ".exclude");
		if (excludes == null)
			return null;
		
		excludeSplit = excludes.split(",");
		excludeSettings = new int[excludeSplit.length];
		
		for (int i = 0; i < excludeSplit.length; i++) {
			excludeSettings[i] = Integer.parseInt(excludeSplit[i].trim());
		}
		
		return excludeSettings;
	}
	
	private DBTable getResult (String key) throws DataSetException {
		DBTable expectedResult;
		String resultString;
		
		resultString = properties.getProperty(key);
		
		if (resultString.trim().equals("")) {
			return null;
		}
		
		expectedResult = DBTableFactory.inst.tableFromString(resultString);
		
		return expectedResult;
	}
	
	public boolean isOrdered (String key) {
		return properties.containsKey(key + ".ordered");
	}
	
	public boolean isError (String key) {
		String query;
		String expected;
		
		query = properties.getProperty(key + ".query");
		expected = properties.getProperty(key + ".result");

		return query.trim().startsWith("ERROR") || expected.trim().equals(""); 
	}
	
	public String getQuery (String key) {
		return properties.getProperty(key + ".query");
	}
		
	protected String[] removeEmptyLines(String[] in) {
		String[] result;
		List<String> buf = new ArrayList<String> ();
		
		for(String line: in) {
			if (!line.trim().equals(""))
				buf.add(line.trim());
		}
		
		result = buf.toArray(new String[] {} );
		
		log.debug("after remove strings: " + Arrays.toString(result));
		
		return result;
	}
	
	public int getNumTest () {
		return numTests;
	}
	
	private int calculateNumTests () {
		int result;
		
		result = 1;
		
		while (properties.getProperty("q" + result + ".query") != null) {
			result++;
		}
		
		return result - 1;
	}
	

}
