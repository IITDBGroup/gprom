/**
 * 
 */
package org.gprom.jdbc.jna;

import java.sql.SQLException;
import java.util.Properties;

import org.apache.logging.log4j.Level;
import org.gprom.jdbc.utility.PropertyWrapper;

/**
 * @author lord_pretzel
 *
 */
public interface GProMJavaInterface {

	public enum DataType {
		DT_STRING,
		DT_INT,
		DT_LONG,
		DT_FLOAT,
		DT_BOOL,
		DT_VARCHAR2
	}
	
	public enum OptionType {
		String,
		Boolean,
		Float,
		Int
	};
	
	public enum ConnectionParam {
		User,
		Host,
		Password,
		Port,
		Database
	}
	
	public enum ExceptionHandler {
		Die,
		Abort,
		Wipe
	}
	
	public enum ExceptionSeverity {
		Recoverable,
		Panic
	}
	
	/* rewrite */
	public String gpromRewriteQuery (String query) throws SQLException;
	
	/* initialization */
	public void init ();
	public void setupOptions (String[] opts);
	public void setupOptions (PropertyWrapper options) throws Exception;
	public void setupPlugins ();
	public void setupFromOptions (String[] opts);
	public void setupFromOptions (PropertyWrapper options) throws Exception;
	public void shutdown();
	
	/* configuration */
    public String getStringOption (String name);
    public int getIntOption (String name);
    public boolean getBoolOption (String name);
    public double getFloatOption (String name);
    
    public void setStringOption (String name, String value);
    public void setIntOption(String name, int value);
    public void setBoolOption(String name, boolean value);
    public void setFloatOption(String name, double value);	

    public void setConnectionOption (PropertyWrapper opts, ConnectionParam p, String value);
    
    public boolean optionExists (String name);
    public OptionType typeOfOption (String name) throws Exception;   
    
    public String optionsHelp();
    
    /* logging */
	public Level intToLogLevel (int in);	
}
