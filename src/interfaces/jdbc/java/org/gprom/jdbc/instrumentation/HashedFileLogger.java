/**
 * 
 */
package org.gprom.jdbc.instrumentation;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.ResultSet;
import java.sql.SQLException;
import org.gprom.jdbc.instrumentation.FunctionsClass;

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
FunctionsClass.printResult(rs, s);
	}

	/* (non-Javadoc)
	 * @see org.grpom.jdbc.instrumentation.IQueryLogger#logQuery(java.lang.String)
	 */
	@Override
	public void logQuery(String s) {
		// TODO Auto-generated method stub
FunctionsClass.queryLogger(s);
	}
	
	public void logHash(String k, String v) throws FileNotFoundException, ClassNotFoundException, IOException{
		FunctionsClass.HashLogger(k, v);
	}

}
