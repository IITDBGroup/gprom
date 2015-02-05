/**
 * 
 */
package org.gprom.jdbc.jna;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;

/**
 * @author lord_pretzel
 *
 */
public interface GProM_JNA extends Library {
	
	// instance of the library
	GProM_JNA INSTANCE = (GProM_JNA) Native.loadLibrary("gprom", 
			GProM_JNA.class);

	// library methods
	// initialization
	public void gprom_init();
	public void gprom_readOptions(int argc, String[] args);
	public void gprom_readOptionAndInit(int argc, String[] args);
	public void gprom_configFromOptions();
	public void gprom_shutdown();
	
	// rewrite methods
    public String gprom_rewriteQuery (String query);
    
    // configuration interface
    public String gprom_getStringOption (String name);
    public int gprom_getIntOption (String name);
    public boolean gprom_getBoolOption (String name);
    public double gprom_getFloatOption (String name);

    public void gprom_setStringOption (String name, String value);
    public void gprom_setIntOption(String name, int value);
    public void gprom_setBoolOption(String name, boolean value);
    public void gprom_setFloatOption(String name, double value);	

    public String gprom_getOptionType(String key);
    public boolean gprom_optionExists(String key);
    
    // logging and error callback interface
    interface GProMLoggerCallbackFunction extends Callback {
        void invoke(String message, String fileInfo, int line, int logLevel);
    }
    
	public void gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback);
	public void gprom_setMaxLogLevel (int maxLevel);
}
