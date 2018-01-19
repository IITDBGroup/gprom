/**
 * 
 */
package org.gprom.jdbc.driver;

import java.sql.DriverPropertyInfo;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.gprom.jdbc.driver.GProMDriverProperties.DriverOptions;

/**
 * @author lord_pretzel
 *
 */
public class GProMDriverProperties {

	public static final String trueValue = "true";
	public static final String falseValue = "false";
	
	// properties
	public static final String GPROM_PREFIX = "gprom.";
	public static final String LOAD_DRIVER_NAME = GPROM_PREFIX + "loadDriver";
	public static final String JDBC_METADATA_LOOKUP_NAME = GPROM_PREFIX + "JDBCMetadataLookup";
	
	public static final String[] trueAndFalse = { "true", "false" };
	
	public enum OptionTyp {
		BOOLEAN,
		STRING,
		INT
	}
	
	public enum DriverOptions {
		
		LOAD_DRIVER(LOAD_DRIVER_NAME,"load the wrapped JDBC driver", trueAndFalse, trueValue, OptionTyp.BOOLEAN),
		JDBC_METADATA_LOOKUP(JDBC_METADATA_LOOKUP_NAME, "use Java metadata lookup", trueAndFalse, trueValue, OptionTyp.BOOLEAN)
		;
		
	    public final String   optionName;
        public final String[] choices;
        public final String   description;
        public final String def;
        public final OptionTyp type;
        public final boolean required;
        
        private DriverOptions(String optionName) {
            this(optionName, null);
        }

        private DriverOptions(String optionName, String[] choices) {
            this(optionName, null, choices, null, OptionTyp.STRING);
        }

        private DriverOptions(String optionName, String description, String[] choices, String def, OptionTyp type) {
        		this(optionName, description, choices, def, type, false);
        }
        
        private DriverOptions(String optionName, String description, String[] choices, String def, OptionTyp type, boolean required) {
            this.optionName = optionName;
            this.description = description;
            this.choices = choices;
            this.def = def;
            this.type = type;
            this.required = false;
        }

	}
	
	// default values
	public static final Map<String,DriverOptions> nameToOpt = createDefaults();
	
	public static Map<String,DriverOptions> createDefaults () {
		HashMap<String,DriverOptions> result = new HashMap<String,DriverOptions> ();
		
		for(DriverOptions o: DriverOptions.values()) {
			result.put(o.optionName, o);
		}
		
		return result;
	}
	
	public static DriverPropertyInfo[] getInfos (Properties prop) {
		DriverPropertyInfo[] result = new DriverPropertyInfo[DriverOptions.values().length];
		
		for(int i = 0; i < DriverOptions.values().length; i++) {
			String val = null;
			DriverOptions o = DriverOptions.values()[i];
			
			if (prop.containsKey(o.optionName)) {
				val = prop.getProperty(o.optionName);
			}
			else {
				if (o.def != null)
					val = o.def;
			}
			
			result[i] = new DriverPropertyInfo(o.optionName, val);
			result[i].description = o.description;
			result[i].choices = o.choices;
			result[i].required = o.required;
		}
		
		return result;
	}
	
	public static String getValueOrDefault (String key, Properties prop) throws Exception {
		if (!nameToOpt.containsKey(key))
			throw new Exception("property " + key + " does not exist");
		
		String val = prop.getProperty(key);
		if (val == null)
			val = nameToOpt.get(key).def;
		return val;
	}
	
	public static Boolean getBoolOptionOrDefault (String key, Properties prop) throws Exception {
		DriverOptions o = nameToOpt.get(key);
		if (!o.type.equals(OptionTyp.BOOLEAN)) {
			throw new Exception ("not a boolean option " + key);
		}
		String val = prop.getProperty(key);
		if (val == null)
			val = nameToOpt.get(key).def;
		
		return Boolean.parseBoolean(val);
	}
	
}
