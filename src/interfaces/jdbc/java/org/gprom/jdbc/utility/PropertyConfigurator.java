/**
 * 
 */
package org.gprom.jdbc.utility;

import java.io.File;
import java.io.FileInputStream;

import org.apache.logging.log4j.core.config.ConfigurationSource;
import org.apache.logging.log4j.core.config.Configurator;


/**
 * @author lord_pretzel
 *
 */
public class PropertyConfigurator {

	
	public static void configureAndWatch(String propFile) throws Exception {
		File file = new File(propFile);
		
		if (!file.exists())
			throw new Exception("can not find logger configuration file " + propFile);
		 
		ConfigurationSource source = new ConfigurationSource(new FileInputStream(file));
		Configurator.initialize(null, source);
	}
	
}
