package org.gprom.jdbc.test.testgenerator;

import java.sql.SQLException;
import java.util.Map.Entry;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.gprom.jdbc.driver.GProMConnection;
import org.gprom.jdbc.test.testgenerator.dataset.DBTable;
import org.gprom.jdbc.test.testgenerator.dataset.DBTableFactory;
import org. gprom.jdbc.test.testgenerator.dataset.DataAndQueryGenerator;
import org.gprom.jdbc.test.testgenerator.dataset.TableCompartor;
import static org.junit.Assert.*;

public class AbstractGProMTester {

	static Logger log = Logger.getLogger(AbstractGProMTester.class);
	
	static protected String path;
	static protected Properties oldProps;
	
	public AbstractGProMTester () {
		
	}
	
	protected static void setUp () throws Exception {
	}

    protected static void tearDown() throws Exception {
    	resetOptions();
    }
	
	/**
	 * @throws Exception 
	 * @throws NumberFormatException 
	 * 
	 */
	protected static void resetOptions() throws NumberFormatException, Exception {
		log.debug("SHOULD RESET GPROM JDBC OPTIONS?");
		if (oldProps != null) {
			GProMConnection g = ConnectionManager.getInstance().getGProMConnection();
			
			for(Entry<?, ?> e: oldProps.entrySet()) {
    			String key = (String) e.getKey();
    			String value = (String) e.getValue();
    			log.debug("reset options: set key " + key + " to " + value);
    			g.getW().setOption(key, value);
    		}
			
			g.getW().reconfPlugins();
		}
	}
    
    /*
     * method to execute test 
     */
    
    protected void testSQLFile (String name) throws SQLException, Exception {
    	DBTable expected;
    	DBTable actualResult = null;
    	String queryString;
    	boolean markedError;
    	DataAndQueryGenerator generator;
    	Properties options;
    	
    	TestInfoHolder.getInstance().setGenerator(name);
    	generator = TestInfoHolder.getInstance().getCurrentGenerator();    	
    	
    	GProMConnection g = ConnectionManager.getInstance().getGProMConnection();
    	
    	options = TestInfoHolder.getInstance().getCurrentGenerator().getOptions();
    	
    	if (options != null)
    	{
    		oldProps = new Properties();
    		for(Entry<?, ?> e: options.entrySet()) {
    			String key = (String) e.getKey();
    			String value = (String) e.getValue();
    			oldProps.setProperty(key, g.getW().getOption(key));
    			log.debug("set key " + key + " to " + value);
    			g.getW().setOption(key, value);
    		}
    		
    		g.getW().reconfPlugins();
    	}
    	else
    		oldProps = null;
    
    	for(int i = 1; i <= generator.getNumTest(); i++) {
    		expected = generator.getExpectedResult("q" + i);
    		queryString = generator.getQuery("q" + i);
    		markedError = generator.isError("q" + i); 
    		
    		logTestResult (name, queryString, i, markedError);
    		
    		if (!markedError) {
    			actualResult = DBTableFactory.inst.tableFromQuery(g, queryString);
	    		assertEquals(actualResult, expected);
    		}
    		
    	}
    }
    
    protected static void setGenerator (String name) throws Exception {
      	DataAndQueryGenerator generator;
    	Properties options;
    	
    	System.out.println("\n********************************************");
    	System.out.println("**   " + name);
    	System.out.println("********************************************\n");
    	
    	TestInfoHolder.getInstance().setGenerator(name);
    	generator = TestInfoHolder.getInstance().getCurrentGenerator();    	
    	
    	GProMConnection g = ConnectionManager.getInstance().getGProMConnection();
    	
    	options = TestInfoHolder.getInstance().getCurrentGenerator().getOptions();
    	
    	if (options != null)
    	{
    		oldProps = new Properties();
    		for(Entry<?, ?> e: options.entrySet()) {
    			String key = (String) e.getKey();
    			String value = (String) e.getValue();
    			oldProps.setProperty(key, g.getW().getOption(key));
    			log.debug("set up <" + name + "> set key " + key + " to " + value);
    			g.getW().setOption(key, value);
    		}
    		
    		g.getW().reconfPlugins();
    	}
    	else
    		oldProps = null;
    }
    
    protected void testSingleQuery (int num) throws Exception {
    	DBTable[] expecteds;
    	DBTable actualResult = null;
    	String queryString;
    	boolean markedError;
    	boolean isOrdered;
    	DataAndQueryGenerator generator;
    	
    	generator = TestInfoHolder.getInstance().getCurrentGenerator();
    	
    	GProMConnection g = ConnectionManager.getInstance().getGProMConnection();
    	
    	Properties options = TestInfoHolder.getInstance().getCurrentGenerator().getOptions();
    	
    	if (options != null && oldProps == null)
    	{
    		oldProps = new Properties();
    		for(Entry<?, ?> e: options.entrySet()) {
    			String key = (String) e.getKey();
    			String value = (String) e.getValue();
    			oldProps.setProperty(key, g.getW().getOption(key));
    			log.debug("set key " + key + " to " + value);
    			g.getW().setOption(key, value);
    		}
    		
    		g.getW().reconfPlugins();
    	}

    	
    	expecteds = generator.getExpectedResults("q" + num);
		queryString = generator.getQuery("q" + num);
		markedError = generator.isError("q" + num); 
		isOrdered = generator.isOrdered("q" + num);
		
		if(isOrdered) {
			for(DBTable t: expecteds) {
				t.setOrdered(true);
			}
		}
		
		logTestResult (TestInfoHolder.getInstance().getGeneratorName() + "\t", queryString, num, markedError);
		
		try {
			if (!markedError) {
				actualResult = DBTableFactory.inst.tableFromQuery(g, queryString);
				actualResult.setOrdered(isOrdered);
	    		assertTrue(TableCompartor.inst.compareTableAgainstMany(actualResult, expecteds));
			}
		}
		catch (Exception e)
		{
			System.out.println("QUERY: " + queryString + "\n\n");
			throw new Exception(queryString, e);
		}
    }
    

    private void logTestResult (String name, String query, int numTest, boolean markedError) {
    	if (markedError) {
    		System.out.println(name + " - " + numTest + ": ERROR MARKED ");
    		TestInfoHolder.getInstance().addMarkedError(name + " - " + numTest + ": ERROR MARKED ");
    	}
    	else {
    		System.out.println(name + " - " + numTest + ": TRY");
    	}
    }

}
