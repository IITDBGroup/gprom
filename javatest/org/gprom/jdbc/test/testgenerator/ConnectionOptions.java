/**
 * 
 */
package org.gprom.jdbc.test.testgenerator;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.utility.SystemOptionReader;
import org.gprom.jdbc.utility.SystemOptionReader.OptionPresence;

import static org.gprom.jdbc.utility.ExceptionUtil.*;

/**
 *
 * ConnectionOptions for GProM jnuit blackbox tests.
 * 
 * @author Boris Glavic
 *
 */
public class ConnectionOptions {

	static Logger log = LogManager.getLogger(ConnectionOptions.class);
	
	private static ConnectionOptions instance;
	private Properties props = new Properties();
	
	public static enum OptionKeys  {
		Host("Host"),
		Port("Port", "1521"),
		DBName("DBName"),
		User("User", "testuser"),
		Password("Password"),
		Backend("Backend", "Oracle"),
		TestcasePath("TestcasePath", "");
		
		public final String name;
		public final String defValue;
		
		private OptionKeys (String name) {
			this(name, null);
		}
		
		private OptionKeys (String name, String defValue) {
			this.name = name;
			this.defValue = defValue;
		}
		
	}
	
	public static enum BackendOptionKeys {
		OracleSID("SID", "Oracle", "orcl"),
		;
		
		public final String name;
		public final String backend; 
		public final String defValue;
		public final boolean optional;
				
		private BackendOptionKeys(String name, String backend) {
			this(name, backend, null);
		}
		
		private BackendOptionKeys(String name, String backend, String defValue) {
			this(name, backend, null, true);
		}
		
		private BackendOptionKeys(String name, String backend, String defValue, boolean optional) {
			this.name = name;
			this.backend = backend;
			this.defValue = defValue;
			this.optional = optional;
		}
		
		public static List<BackendOptionKeys> getBackendKeys (String backend) {
			List<BackendOptionKeys> result = new ArrayList<BackendOptionKeys> ();
			
			for(BackendOptionKeys k: BackendOptionKeys.values()) {
				if (k.backend.equals(backend))
					result.add(k);
			}
			
			return result;
		}
		
		public static BackendOptionKeys getOption (String backend, String key) {
			for(BackendOptionKeys k: BackendOptionKeys.values()) {
				if (k.name.equals(key) && k.backend.equals(backend))
					return k;
			}
			return null;
		}
		
		public String key() {
			return backend + "." + name;
		}
	}
	
	private static final String GPROM_ENVIRONMENT_PREFIX = "GPROM_AUTOTEST.";
	private static final String GPROM_KEY_OPTIONDIR = "optiondir";
	private boolean initialized = false;
	
	
	private ConnectionOptions () throws Exception {
	}
	
	public void init (String dir) throws Exception {
		props = new Properties ();
		
		readConfigFile(dir);
		readEnvAndSystemProperties();
	}
	
	public void readConfigFile(String dir) throws IOException {		
		if (dir == null)
			log.error("configuration file directory not set: set environment variable {}{} or use -D{}", 
					GPROM_ENVIRONMENT_PREFIX, GPROM_KEY_OPTIONDIR, GPROM_KEY_OPTIONDIR);
		
		File pFile = new File(dir + "/options.txt");
		if (!pFile.exists()) {
			log.error("user specified dir {}, but did not find options.txt in this directory", dir);
			throw new FileNotFoundException(pFile.toString());
		}
		props.load(new FileInputStream(pFile));		
	}
	
	public void readEnvAndSystemProperties () throws Exception {
		for(OptionKeys k: OptionKeys.values()) {
			String key = k.toString();
			String value = SystemOptionReader.inst.getEnvOrProperty(GPROM_ENVIRONMENT_PREFIX, key);
			// to make life easier in build.xml
			if (value != null
					&& value.trim().isEmpty() 
					&& SystemOptionReader.inst.getOptPresence(GPROM_ENVIRONMENT_PREFIX, key).equals(OptionPresence.Both)) {
				value = SystemOptionReader.inst.getEnv(GPROM_ENVIRONMENT_PREFIX, key);
			}

			if (value != null)
				props.setProperty(key, value);
		}
		
		checkProperties();
		
		for(BackendOptionKeys k: BackendOptionKeys.getBackendKeys(getBackend())) {
			String key = k.key();
			
			String value = SystemOptionReader.inst.getEnvOrProperty(GPROM_ENVIRONMENT_PREFIX, key);
			// to make life easier in build.xml
			if (value != null
					&& value.trim().isEmpty() 
					&& SystemOptionReader.inst.getOptPresence(GPROM_ENVIRONMENT_PREFIX, key).equals(OptionPresence.Both)) {
				value = SystemOptionReader.inst.getEnv(GPROM_ENVIRONMENT_PREFIX, key);
			}

			if (value != null)
				props.setProperty(key, value);
		}
		
		checkBackendProperties();
	}
	
	private void checkProperties () throws Exception {
		for(OptionKeys k: OptionKeys.values()) {
			String key = k.toString();
			
			if(!props.containsKey(key) && k.defValue != null)
				props.setProperty(key, k.defValue);
			
			if (!props.containsKey(key) && ! k.equals(OptionKeys.TestcasePath))
				throw new Exception("need connection option " + key + " to be set to run tests") ;
		}		
	}
	
	private void checkBackendProperties () throws Exception {
		for(BackendOptionKeys k: BackendOptionKeys.getBackendKeys(getBackend())) {
			String key = k.key();
					
			if (!k.optional) {
				if (!props.containsKey(key))
					throw formatMessageException("backend %s need option %s", getBackend(), key);  
			}
		}
	}
	
	public static ConnectionOptions getInstance () throws Exception {
		if (instance == null) { 
			instance = new ConnectionOptions ();
		}
		
		return instance;
	}
	
	public String getBackend() {
		return props.getProperty("Backend");
	}
	
	public String getBackendOption (BackendOptionKeys k) {
		return props.getProperty(k.key()); 
	}
	
	public String getBackendOption (String key) {
		BackendOptionKeys k = BackendOptionKeys.getOption(getBackend(), key);
		if (k == null)
			return null;
		String value = props.getProperty(k.key()); 
		return value;
	}
	
	public String getHost () {
		return props.getProperty("Host");
	}
	
	public int getPort () {
		return Integer.valueOf(props.getProperty("Port"));
	}
	
	public String getDbName () {
		return props.getProperty("DBName");
	}
	
	public String getUser () {
		System.out.println(props.get("User"));
		return props.getProperty("User");
	}
	
	public String getPassword () {
		return props.getProperty("Password");
	}
	
	public String getSchema () {
		return props.getProperty("Schema");
	}
	
	public String getPath () {
		return props.getProperty("TestcasePath");
	}
	
	public void setPath (String path) throws Exception {
		File dir = new File(path);
		if (dir.exists()) {
			log.info("switching test case path to {}", path);
		}
		if (!initialized) {
			String dirEnv = SystemOptionReader.inst.getEnvOrProperty(GPROM_ENVIRONMENT_PREFIX, GPROM_KEY_OPTIONDIR);
			if (dirEnv == null)
				dirEnv = path;
			else
				log.info("user passed options path {}, use this to initialize", dirEnv);
			init(dirEnv);
		}
		if (! props.containsKey(OptionKeys.TestcasePath.toString())) {
			log.info("TestcasePath has not been set yet, set it to {}" , path);
			props.setProperty(OptionKeys.TestcasePath.toString(), path);
		}
		log.error("options are: {}", props);
	}
}
