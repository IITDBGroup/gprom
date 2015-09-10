/**
 * 
 */
package org.gprom.jdbc.instrumentation;

import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * @author lord_pretzel
 *
 */
public class HashedFileLogger implements IInstrumentationLogger {

	/* (non-Javadoc)
	 * @see org.grpom.jdbc.instrumentation.IProvenanceLogger#printResult(java.sql.ResultSet, java.lang.String)
	 */
	@Override
	public void printResult(ResultSet rs, String s) throws SQLException {
		// TODO Auto-generated method stub

	}

	/* (non-Javadoc)
	 * @see org.grpom.jdbc.instrumentation.IQueryLogger#logQuery(java.lang.String)
	 */
	@Override
	public void logQuery(String s) {
		// TODO Auto-generated method stub

	}

}
