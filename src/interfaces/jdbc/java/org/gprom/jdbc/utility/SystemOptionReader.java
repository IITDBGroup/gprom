/**
 * 
 */
package org.gprom.jdbc.utility;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * @author lord_pretzel
 *
 */
public class SystemOptionReader {

	static Logger log = LogManager.getLogger(SystemOptionReader.class);
	
	public static final SystemOptionReader inst = new SystemOptionReader();
	
	public static enum OptionPresence {
		Nowhere,
		Environment,
		Property,
		Both
	}
	
	public String getEnvOrProperty (String envPrefix, String key) {
		String envVar = envPrefix + key;
		String value = null;
		String result = null;
		
		// check environment variables first
		value = System.getenv(envVar);
		if (value != null) {
			log.debug("read environment variable {}: <{}>", envVar, value);
			result = value;
		}

		// properties override environment variables
		value = System.getProperty(key);
		if (value != null) {
			log.debug("read property {}: <{}>", key, value);
			result = value;
		}
		
		return result;
	}
	
	public String getEnv (String envPrefix, String key) {
		String envVar = envPrefix + key;
		String value = null;
		String result = null;
		
		// check environment variables first
		value = System.getenv(envVar);
		if (value != null) {
			log.debug("read environment variable {}: <{}>", envVar, value);
			result = value;
		}
		
		return result;		
	}

	public String getSystem (String key) {
		String value = null;
		String result = null;
		
		// properties override environment variables
		value = System.getProperty(key);
		if (value != null) {
			log.debug("read property {}: <{}>", key, value);
			result = value;
		}
		
		return result;
	}

	
	public OptionPresence getOptPresence (String envPrefix, String key) {
		boolean systemPresence = false;
		boolean envPresence = false;
		String envVar = envPrefix + key;		
		String value;
		
		value = System.getenv(envVar);
		if (value != null) {
			systemPresence = true;
		}
		
		value = System.getProperty(key);
		if (value != null) {
			envPresence = true;
		}
		
		if (systemPresence && !envPresence)
			return OptionPresence.Property;
		if (!systemPresence && envPresence)
			return OptionPresence.Environment;
		if (systemPresence && envPresence)
			return OptionPresence.Both;
		// both false
		return OptionPresence.Nowhere;
	}

	
}
