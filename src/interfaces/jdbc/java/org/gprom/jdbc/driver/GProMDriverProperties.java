/**
 * 
 */
package org.gprom.jdbc.driver;

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

/**
 * @author lord_pretzel
 *
 */
public class GProMDriverProperties {

	public static final String trueValue = "TRUE";
	
	// properties
	public static final String LOAD_DRIVER = "gprom.loadDriver";
	public static final String JDBC_METADATA_LOOKUP = "gprom.JDBCMetadataLookup";
	
	// default values
	public static final Map<String,String> defaults = createDefaults();
	
	public static Map<String,String> createDefaults () {
		HashMap<String,String> result = new HashMap<String,String> ();
		
		result.put(LOAD_DRIVER, trueValue);
		result.put(JDBC_METADATA_LOOKUP, trueValue);
		
		return result;
	}
	
	public static String getValueOrDefault (String key, Properties prop) {
		String val = prop.getProperty(key);
		if (val == null)
			val = defaults.get(key);
		return val;
	}
	
}
