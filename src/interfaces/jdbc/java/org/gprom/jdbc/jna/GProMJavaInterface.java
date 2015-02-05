/**
 * 
 */
package org.gprom.jdbc.jna;

import java.sql.SQLException;

import org.apache.log4j.Level;

/**
 * @author lord_pretzel
 *
 */
public interface GProMJavaInterface {

	public enum OptionType {
		String,
		Boolean,
		Float,
		Int
	};
	
	/* rewrite */
	public String gpromRewriteQuery (String query) throws SQLException;
	
	/* initialization */
	public void init ();
	public void setupOptions (String[] opts);
	public void setupPlugins ();
	public void setupFromOptions (String[] opts);
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

    public boolean optionExists (String name);
    public OptionType typeOfOption (String name) throws Exception;
    
    /* logging */
	public Level intToLogLevel (int in);	
}
