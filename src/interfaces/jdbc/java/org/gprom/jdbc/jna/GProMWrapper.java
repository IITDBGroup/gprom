/**
 * 
 */
package org.gprom.jdbc.jna;

import java.sql.SQLException;
import java.util.Properties;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.gprom.jdbc.jna.GProMJavaInterface;
import org.gprom.jdbc.jna.GProMJavaInterface.ConnectionParam;
import org.gprom.jdbc.utility.PropertyWrapper;

import com.sun.jna.Pointer;

/**
 * @author lord_pretzel
 *
 */
public class GProMWrapper implements GProMJavaInterface {

	static Logger libLog = Logger.getLogger("LIBGPROM");


	public static final String KEY_CONNECTION_HOST = "connection.host";
	public static final String KEY_CONNECTION_DATABASE = "connection.db";
	public static final String KEY_CONNECTION_USER = "connection.user";
	public static final String KEY_CONNECTION_PASSWORD = "connection.passwd";
	public static final String KEY_CONNECTION_PORT = "connection.port";
	
	private GProM_JNA.GProMLoggerCallbackFunction callbackReference;
	
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
		Pointer p =  GProM_JNA.INSTANCE.gprom_rewriteQuery(query);
		String result = p.getString(0).replaceFirst(";\\s+\\z", "");
		libLog.info("HAVE REWRITTEN:\n\n" + query + "\n\ninto:\n\n" + result);
		return result;
	}

	public void init () {
		callbackReference = new GProM_JNA.GProMLoggerCallbackFunction () {
			public void invoke(String message, String file, int line, int level) {
				logCallback(message, file, line, level);
			}
		};

		GProM_JNA.INSTANCE.gprom_registerLoggerCallbackFunction(callbackReference);
		GProM_JNA.INSTANCE.gprom_init();
		GProM_JNA.INSTANCE.gprom_setMaxLogLevel(4);
	}

	public void setLogLevel (int level)
	{
		GProM_JNA.INSTANCE.gprom_setBoolOption("log.active", true);
		GProM_JNA.INSTANCE.gprom_setIntOption("log.level", level);
		GProM_JNA.INSTANCE.gprom_setMaxLogLevel(level);
	}

//	public void setupOptions (Properties opts)
//	{
//		for(Object key: opts.keySet())
//		{
//			String k = (String) key;
//			setStringOption(k, opts.getProperty(k));
//		}
//	}

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

	public void setupFromOptions (PropertyWrapper options) throws Exception {
		setupOptions(options);
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

	public void setOption (String key, String value) throws NumberFormatException, Exception {
		switch(typeOfOption(key)) {
		case Boolean:
			setBoolOption(key, Boolean.getBoolean(value));
			break;
		case Float:
			setFloatOption(key, Float.valueOf(value));
			break;
		case Int:
			setIntOption(key, Integer.valueOf(value));
			break;
		case String:
			setStringOption(key, value);
			break;
		default:
			break;
		}
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

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#optionExists(java.lang.String)
	 */
	@Override
	public boolean optionExists(String name) {
		// TODO Auto-generated method stub
		return GProM_JNA.INSTANCE.gprom_optionExists(name);
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#typeOfOption(java.lang.String)
	 */
	@Override
	public OptionType typeOfOption(String name) throws Exception {
		if (GProM_JNA.INSTANCE.gprom_optionExists(name))
		{
			String optionType = GProM_JNA.INSTANCE.gprom_getOptionType(name);
			if(optionType.equals("OPTION_STRING"))
				return OptionType.String;
			if(optionType.equals("OPTION_BOOL"))
				return OptionType.Boolean;
			if(optionType.equals("OPTION_FLOAT"))
				return OptionType.Float;
			if(optionType.equals("OPTION_INT"))
				return OptionType.Int;
		}
		throw new Exception("option " + name + " does is not a valid option");
	}

	/** 
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#setOptions(java.util.Properties)
	 */
	@Override
	public void setupOptions(PropertyWrapper options) throws Exception {
		for (String key: options.stringPropertyNames()) {
			libLog.debug("key: "+ key + " type: " + typeOfOption(key));
			
			switch(typeOfOption(key)) {
			case Boolean:
				setBoolOption(key, options.getBool(key));
				break;
			case Float:
				setFloatOption(key, options.getFloat(key));
				break;
			case Int:
				setIntOption(key, options.getInt(key));
				break;
			case String:
				setStringOption(key, options.getString(key));
				break;
			default:
				break;
			}
		}
	}


	public void setConnectionOption (PropertyWrapper opts, ConnectionParam p, String value) {
		switch(p)
		{
		case Database:
			libLog.debug("set key " + KEY_CONNECTION_DATABASE + " to " + value);
			opts.setProperty(KEY_CONNECTION_DATABASE, value);
			break;
		case Host:
			libLog.debug("set key " + KEY_CONNECTION_HOST + " to " + value);
			opts.setProperty(KEY_CONNECTION_HOST, value);
			break;
		case Password:
			libLog.debug("set key " + KEY_CONNECTION_PASSWORD + " to " + value);
			opts.setProperty(KEY_CONNECTION_PASSWORD, value);
			break;
		case Port:
			libLog.debug("set key " + KEY_CONNECTION_PORT + " to " + value);
			opts.setProperty(KEY_CONNECTION_PORT, value);
			break;
		case User:
			libLog.debug("set key " + KEY_CONNECTION_USER + " to " + value);
			opts.setProperty(KEY_CONNECTION_USER, value);
			break;
		default:
			break;
		}
	}
	
	
}
