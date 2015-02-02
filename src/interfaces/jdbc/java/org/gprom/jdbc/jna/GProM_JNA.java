/**
 * 
 */
package org.gprom.jdbc.jna;

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
	// rewrite methods
    public String gprom_rewriteQuery (String query);
    
    // configuration interface
//	public void setOption (String key, String value);
//	public boolean getBoolProp (String key);
	
	// error callback interface
}
