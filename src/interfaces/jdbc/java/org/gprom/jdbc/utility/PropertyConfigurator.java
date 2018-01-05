/**
 * 
 */
package org.gprom.jdbc.utility;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URL;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.core.Filter;
import org.apache.logging.log4j.core.LoggerContext;
import org.apache.logging.log4j.core.appender.ConsoleAppender;
import org.apache.logging.log4j.core.config.ConfigurationFactory;
import org.apache.logging.log4j.core.config.ConfigurationSource;
import org.apache.logging.log4j.core.config.Configurator;
import org.apache.logging.log4j.core.config.builder.api.AppenderComponentBuilder;
import org.apache.logging.log4j.core.config.builder.api.ConfigurationBuilder;
import org.apache.logging.log4j.core.config.builder.api.ConfigurationBuilderFactory;
import org.apache.logging.log4j.core.config.builder.impl.BuiltConfiguration;
import org.apache.logging.log4j.core.config.xml.XmlConfigurationFactory;
import org.gprom.jdbc.jna.GProMNativeLibraryLoader;


/**
 * @author lord_pretzel
 *
 */
public class PropertyConfigurator {
	
	public static final String PROP_LOG_CONFIG_FILE = "log4j2.configurationFile";
	public static final String PROP_LOG_LEVEL = "log4j2.level";
	
	public static void configureHonoringProperties (String ...xmlFiles) {
		String setConfFile = SystemOptionReader.inst.getEnvOrProperty("", PROP_LOG_CONFIG_FILE);
		String logLevel = SystemOptionReader.inst.getEnvOrProperty("", PROP_LOG_LEVEL);
		boolean foundFile = false;
		
		if (setConfFile != null) {
			LogManager.getRootLogger().error("keep configuration from {}", setConfFile);
			return;
		}
		if (logLevel != null)
		{
			Level userLevel = Level.valueOf(logLevel);
			configureDefaultConsoleLogger(userLevel);
			LogManager.getRootLogger().error("use default logger with log level {}",  userLevel);
			return;
		}
		for(String xmlFile: xmlFiles)
		{	
			if(!foundFile) {
				try {
					configureAndWatch(xmlFile);
					foundFile=true;
				} catch (Exception e) {
				}
			}
		}
		// fallback is to configure default logger with debug log level to expose problems
		if (!foundFile)
			configureDefaultConsoleLogger(Level.DEBUG);
	}
	
	public static void configureAndWatch(String propFile) throws Exception {
		// try to load from classpath first
		String path = "/" + propFile;
		InputStream in = PropertyConfigurator.class.getResourceAsStream(path);
		
		if (in != null) {
			configureFromURL(in, path);
			return;
		}
		
		// else try to load from file
		File file = new File(propFile);
		
		if (!file.exists())
			throw new Exception("can not find logger configuration file " + propFile);
		 
		configureFromFile(file);
	}
	
	private static void configureFromURL (InputStream in, String path) throws IOException {
		ConfigurationFactory configFactory = XmlConfigurationFactory.getInstance();
        ConfigurationFactory.setConfigurationFactory(configFactory);
        LoggerContext ctx = (LoggerContext) LogManager.getContext(false);
        ConfigurationSource configurationSource = new ConfigurationSource(in);

        ctx.start(configFactory.getConfiguration(ctx, configurationSource));
        LogManager.getRootLogger().error("recofigured from resource {}", path);
	}
	
	private static void configureFromFile(File f) throws IOException {
		ConfigurationFactory configFactory = XmlConfigurationFactory.getInstance();
        ConfigurationFactory.setConfigurationFactory(configFactory);
        LoggerContext ctx = (LoggerContext) LogManager.getContext(false);
        InputStream inputStream = new FileInputStream(f);
        ConfigurationSource configurationSource = new ConfigurationSource(inputStream);

        ctx.start(configFactory.getConfiguration(ctx, configurationSource));
        LogManager.getRootLogger().error("recofigured from config file {}", f);
	}
	
	public static void configureWithDefaultAsFallback (String propertyFile) {
		File file = new File(propertyFile);
		Exception keepE = null;
		
		if (!file.exists()) {
			configureDefaultConsoleLogger(Level.DEBUG);
			LogManager.getRootLogger().error("did not find log4j configuration file {}, use default configuration", propertyFile);
		}
		else
			try {
				configureAndWatch(propertyFile);
				LogManager.getRootLogger().info("configured log4j2 from {}", propertyFile);
			}
			catch (Exception e) {
				configureDefaultConsoleLogger(Level.DEBUG);
				LogManager.getRootLogger().error("tried to load log4j2 configuration {}, but failed", propertyFile);
				LoggerUtil.logException(e, LogManager.getRootLogger());
				return;
			}
	}
	
	public static void configureDefaultConsoleLogger(Level logLevel) {
		ConfigurationFactory configFactory = GProMLog4j2ConfigurationFactory.GProMConfigurationFactory.inst;
		GProMLog4j2ConfigurationFactory.GProMConfigurationFactory.inst.setLogLevel(logLevel);
        ConfigurationFactory.setConfigurationFactory(configFactory);
        LoggerContext ctx = (LoggerContext) LogManager.getContext(false);
        System.setProperty(PROP_LOG_LEVEL, logLevel.name());
        ctx.start(configFactory.getConfiguration(ctx, ConfigurationSource.NULL_SOURCE));

        LogManager.getRootLogger().error("cofigured to log to standard ourput with loglevel {}", logLevel);
	}
	
}
