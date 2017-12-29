/**
 * 
 */
package org.gprom.jdbc.utility;

import java.io.File;
import java.io.FileInputStream;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.core.Filter;
import org.apache.logging.log4j.core.LoggerContext;
import org.apache.logging.log4j.core.appender.ConsoleAppender;
import org.apache.logging.log4j.core.config.ConfigurationSource;
import org.apache.logging.log4j.core.config.Configurator;
import org.apache.logging.log4j.core.config.builder.api.AppenderComponentBuilder;
import org.apache.logging.log4j.core.config.builder.api.ConfigurationBuilder;
import org.apache.logging.log4j.core.config.builder.api.ConfigurationBuilderFactory;
import org.apache.logging.log4j.core.config.builder.impl.BuiltConfiguration;


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
		LoggerContext ctx = Configurator.initialize(null, source);
		ctx.updateLoggers();
	}
	
	public static void configureWithDefaultAsFallback (String propertyFile) {
		File file = new File(propertyFile);
		Exception keepE = null;
		
		if (!file.exists()) {
			configureDefaultConsoleLogger();
			LogManager.getRootLogger().error("did not find log4j configuration file {}, use default configuration", propertyFile);
		}
		else
			try {
				configureAndWatch(propertyFile);
				LogManager.getRootLogger().info("configured log4j2 from {}", propertyFile);
			}
			catch (Exception e) {
				configureDefaultConsoleLogger();
				LogManager.getRootLogger().error("tried to load log4j2 configuration {}, but failed", propertyFile);
				LoggerUtil.logException(e, LogManager.getRootLogger());
				return;
			}
	}
	
	public static void configureDefaultConsoleLogger() {
		ConfigurationBuilder<BuiltConfiguration> builder = ConfigurationBuilderFactory.newConfigurationBuilder();
		builder.setStatusLevel(Level.ERROR);
		builder.setConfigurationName("BuilderTest");
		builder.add(builder.newFilter("ThresholdFilter", Filter.Result.ACCEPT, Filter.Result.NEUTRAL)
		    .addAttribute("level", Level.DEBUG));
		AppenderComponentBuilder appenderBuilder = builder.newAppender("Stdout", "CONSOLE").addAttribute("target",
		    ConsoleAppender.Target.SYSTEM_OUT);
		appenderBuilder.add(builder.newLayout("PatternLayout")
		    .addAttribute("pattern", "%d [%t] %-5level: %msg%n%throwable"));
		appenderBuilder.add(builder.newFilter("MarkerFilter", Filter.Result.DENY, Filter.Result.NEUTRAL)
		    .addAttribute("marker", "FLOW"));
		builder.add(appenderBuilder);
		builder.add(builder.newLogger("org.apache.logging.log4j", Level.DEBUG)
		    .add(builder.newAppenderRef("Stdout")).addAttribute("additivity", false));
		builder.add(builder.newRootLogger(Level.DEBUG).add(builder.newAppenderRef("Stdout")));
		LoggerContext ctx = Configurator.initialize(builder.build());
		ctx.updateLoggers();
	}
	
}
