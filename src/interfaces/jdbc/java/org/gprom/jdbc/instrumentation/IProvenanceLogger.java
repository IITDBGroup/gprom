/**
 * 
 */
package org.gprom.jdbc.instrumentation;

import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * Interface for logging provenance of queries used by the instrumented of the GProM JDBC driver.
 * 
 * @author lord_pretzel
 *
 */
public interface IProvenanceLogger {

	public void printResult(ResultSet rs, String s) throws SQLException;
	
}
