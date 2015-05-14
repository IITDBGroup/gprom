/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.StringReader;
import java.util.InvalidPropertiesFormatException;
import java.util.Properties;
import java.util.Vector;
import java.util.logging.Logger;

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
	
	public ITable getExpectedResult (String key) throws DataSetException, IOException {
		XmlDataSet result;
		
		result = getResult (key + ".result");
		return result.getTable("transformed");
	}
	
	public ITable[] getExpectedResults (String key) throws DataSetException {
		Vector<XmlDataSet> expectedResults;
		ITable[] result;
		String resultKey;
		int num;
		
		 expectedResults = new Vector<XmlDataSet> ();
		
		expectedResults.add (getResult(key + ".result"));
		
		num = 1;
		resultKey = key + ".result" + num;
		while((properties.getProperty(resultKey)) != null) {
			expectedResults.add (getResult(resultKey));
			num++;
			resultKey = key + ".result" + num;
		}
		
		result = new ITable[expectedResults.size()];
		for (int i = 0; i < result.length; i++) {
			result[i] = expectedResults.get(i).getTable("transformed");
		}
		
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
	
	private XmlDataSet getResult (String key) throws DataSetException {
		XmlDataSet expectedResult;
		String resultString;
		String xmlString ="";
		
		resultString = properties.getProperty(key);
		
		if (resultString.trim().equals("")) {
			return null;
		}
		
		xmlString = transformStringResultSetToXML (resultString);
		expectedResult = new XmlDataSet(new StringReader(xmlString));
		
		//XmlDataSet.write(expectedResult, System.out);
		
		return expectedResult;
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
	
	private String transformStringResultSetToXML (String resultSet) {
		String[] lines;
		String[] columns;
		String[] rows;
		int numColumns;
		StringBuffer result;
		String value;
		
		result = new StringBuffer();
		result.append(header);
		
		/* split lines */
		lines = resultSet.split("\n");
		
		/* get columns */
		columns = lines[1].split("\\|");
		numColumns = columns.length;
		
		/* output columns */
		for (int i = 0; i < numColumns; i++) {
			result.append("\t\t<column>" + columns[i].trim() + "</column>\n");
		}
		
		/* get lines and output them */
		for (int i = 3; i < lines.length; i++) {
			rows = lines[i].split("\\|");
			
			/* replace escapted '|' characters */
			for(int j = 0; j < numColumns; j++) {
				rows[j] = rows[j].replaceAll("\\$MID\\$", "|");
			}
			
			result.append("\t\t<row>\n\t\t\t");
			
			for (int j = 0; j < rows.length; j++) {
				if (rows[j].trim().equals("")) {
					result.append("<null></null>");
				}
				else if (rows[j].trim().equals("EMPTYSTRING")) {
					result.append("<value></value>");
				}
				else {
					value = rows[j].trim();
					value = escapeXml(value);
					if(value.startsWith("@"))
						value = value.replace('@', ' ');
					result.append("<value>" + value + "</value>");
				}
			}
			result.append("\n\t\t</row>\n");
		}
		
		
		result.append(footer);
		
		//System.out.println(result.toString());
		
		return result.toString();
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
	
	private String escapeXml (String xml) {
		StringBuilder result;
		char[] chars;
		
		result = new StringBuilder();
		chars = xml.toCharArray();
		
		for(int i = 0; i < chars.length; i++) {
			if (chars[i] == '<')
				result.append("&lt;");
			else if (chars[i] == '>')
				result.append("&gt;");
			else
				result.append(chars[i]);
		}		
		
		return result.toString();
	}
}
