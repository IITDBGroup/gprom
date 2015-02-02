/**
 * 
 */
package org.gprom.jdbc.jna;

import java.sql.SQLException;
import java.util.Properties;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.gprom.jdbc.jna.GProMJavaInterface;

/**
 * @author lord_pretzel
 *
 */
public class GProMWrapper implements GProMJavaInterface {

	static Logger libLog = Logger.getLogger("LIBGPROM");
	
	
	public static GProMWrapper inst = new GProMWrapper ();
	
	public static GProMWrapper getInstance () {
		return inst;
	}
	
	private GProMWrapper () {
		
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#gpromRewriteQuery(java.lang.String)
	 */
	@Override
	public String gpromRewriteQuery(String query) throws SQLException {
		return GProM_JNA.INSTANCE.gprom_rewriteQuery(query);
	}

	public void init () {
		GProM_JNA.GProMLoggerCallbackFunction callback = new GProM_JNA.GProMLoggerCallbackFunction () {
		    public void invoke(String message, String file, int line, int level) {
		    	System.out.println("invoke");
		        logCallback(message, file, line, level);
		    }
		};
		
		GProM_JNA.INSTANCE.gprom_registerLoggerCallbackFunction(callback);
		GProM_JNA.INSTANCE.gprom_init();
		GProM_JNA.INSTANCE.gprom_setMaxLogLevel(0);
	}

	public void setLogLevel (int level)
	{
		GProM_JNA.INSTANCE.gprom_setIntOption("log.level", level);
	}
	
	public void setupOptions (Properties opts)
	{
		for(Object key: opts.keySet())
		{
			String k = (String) key;
			setStringOption(k, opts.getProperty(k));
		}
	}
	
	public void setupOptions (String[] opts)
	{
		GProM_JNA.INSTANCE.gprom_readOptions(opts.length, opts);
	}
	
	public void setupPlugins ()
	{
		GProM_JNA.INSTANCE.gprom_configFromOptions();
		
	}
	
	public void setupFromOptions (String[] opts)
	{
		setupOptions(opts);
		setupPlugins();
	}
	
	public void shutdown()
	{
		GProM_JNA.INSTANCE.gprom_shutdown();
	}
	
	private void logCallback (String message, String file, int line, int level) {
		String printMes = file + " at " + line + ": " + message;
		
		libLog.log(intToLogLevel(level), printMes);
	}
	
	public Level intToLogLevel (int in)
	{
		if (in == 0 || in == 1)
			return Level.FATAL;
		if (in == 2)
			return Level.ERROR;
		if (in == 3)
			return Level.INFO;
		if (in == 4)
			return Level.DEBUG;
		
		return Level.DEBUG;
	}
	
    public String getStringOption (String name)
    {
    	return GProM_JNA.INSTANCE.gprom_getStringOption(name);
    }
    
    public int getIntOption (String name)
    {
    	return GProM_JNA.INSTANCE.gprom_getIntOption(name);	
    }
    
    public boolean getBoolOption (String name)
    {
    	return GProM_JNA.INSTANCE.gprom_getBoolOption(name);
    }
    
    public double getFloatOption (String name)
    {
    	return GProM_JNA.INSTANCE.gprom_getFloatOption(name);
    }

    public void setStringOption (String name, String value)
    {
    	GProM_JNA.INSTANCE.gprom_setStringOption(name, value);
    }
    
    public void setIntOption(String name, int value)
    {
    	GProM_JNA.INSTANCE.gprom_setIntOption(name, value);
    }
    
    public void setBoolOption(String name, boolean value)
    {
    	GProM_JNA.INSTANCE.gprom_setBoolOption(name, value);
    }
    
    public void setFloatOption(String name, double value)
    {
    	GProM_JNA.INSTANCE.gprom_setFloatOption(name, value);
    }
	
	
}
