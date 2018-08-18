/**
 * 
 */
package org.gprom.jdbc.utility;

import java.io.Serializable;
import java.net.URI;
import java.nio.charset.Charset;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.core.Appender;
import org.apache.logging.log4j.core.Layout;
import org.apache.logging.log4j.core.LoggerContext;
import org.apache.logging.log4j.core.appender.ConsoleAppender;
import org.apache.logging.log4j.core.config.Configuration;
import org.apache.logging.log4j.core.config.ConfigurationFactory;
import org.apache.logging.log4j.core.config.ConfigurationSource;
import org.apache.logging.log4j.core.config.DefaultConfiguration;
import org.apache.logging.log4j.core.config.LoggerConfig;
import org.apache.logging.log4j.core.config.Order;
import org.apache.logging.log4j.core.config.plugins.Plugin;
import org.apache.logging.log4j.core.layout.PatternLayout;

/**
 * @author lord_pretzel
 *
 */
public class GProMLog4j2ConfigurationFactory {

    public static final String GRPOM_LOGGER_NAME = "gprom.loggerconfig";
    public static final String CONSOLE_PATTERN_LAYOUT = "%n[%d{yyyy-MM-dd HH:mm:ss}] [%-5p] [%l]%n\t%m%n%n";

    
    
    /**
     * Just to make JVM visit this class to initialize the static parts.
     */
    public static void configure() {
    }

    @Plugin(category = ConfigurationFactory.CATEGORY, name = "GProMConfigurationFactory")
    @Order(15)
    public static class GProMConfigurationFactory  extends ConfigurationFactory {
        public static final String[] SUFFIXES = new String[] {".json", "*"};

        public static final GProMConfigurationFactory inst = new GProMConfigurationFactory();
        
        private Level logLevel = Level.DEBUG;
        
        @Override
        protected String[] getSupportedTypes() {
            return SUFFIXES;
        }

        @Override
        public Configuration getConfiguration(LoggerContext arg0, ConfigurationSource arg1) {
            return new Log4j2Configuration(logLevel);
        }

        @Override
        public Configuration getConfiguration(LoggerContext arg0, String arg1, URI arg2) {
            return new Log4j2Configuration(logLevel);
        }
        
        @Override
        public Configuration getConfiguration(LoggerContext arg0, String arg1, URI arg2, ClassLoader l) {
            return new Log4j2Configuration(logLevel);
        }

		public Level getLogLevel() {
			return logLevel;
		}

		public void setLogLevel(Level logLevel) {
			this.logLevel = logLevel;
		}
    
    }

    private static class Log4j2Configuration extends DefaultConfiguration {

        public Log4j2Configuration(Level l) {
            super.doConfigure();
            setName("gprom-log4j2");

            // LOGGERS
            //      com.websitester
//            AppenderRef[] refs = new AppenderRef[] {};
//            Property[] properties = new Property[] {};
//            LoggerConfig websitesterLoggerConfig = LoggerConfig.createLogger(true, Level.INFO, GRPOM_LOGGER_NAME, "true", refs, properties, this, null);
//            addLogger(GRPOM_LOGGER_NAME, websitesterLoggerConfig);


            // APPENDERS
            final Charset charset = Charset.forName("UTF-8");
            
            Layout<? extends Serializable> consoleLayout = PatternLayout.newBuilder()
                    .withPattern(CONSOLE_PATTERN_LAYOUT)
                    .withPatternSelector(null)
                    .withConfiguration(this)
                    .withRegexReplacement(null)
                    .withCharset(charset)
                    .withAlwaysWriteExceptions(isShutdownHookEnabled)
                    .withNoConsoleNoAnsi(isShutdownHookEnabled)
                    .withHeader(null)
                    .withFooter(null)
                    .build();
            Appender consoleAppender = ConsoleAppender.newBuilder()
                    .setConfiguration(this)
                    .withFilter(null)
                    .withIgnoreExceptions(true)
                    .withImmediateFlush(true)
                    .withLayout(consoleLayout)
                    .withName("gprom_basic_console")
                    .setTarget(ConsoleAppender.Target.SYSTEM_OUT)
                    .build();
            consoleAppender.start();
            addAppender(consoleAppender);
            LoggerConfig r = getRootLogger();
            r.setLevel(l);
            for (String a: getRootLogger().getAppenders().keySet()) {
            		r.removeAppender(a);
            }
            getRootLogger().addAppender(consoleAppender, Level.DEBUG, null);
        }
    }
}
