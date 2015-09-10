package org.gprom.jdbc.instrumentation;

/**
 * Interface defining methods for logging queries implemented by classes used by the JDBC instrumentation.
 * @author lord_pretzel
 *
 */
interface IQueryLogger {
	
	public void logQuery(String s);
}
