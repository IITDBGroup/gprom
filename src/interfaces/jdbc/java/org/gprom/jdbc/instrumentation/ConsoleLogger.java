/**
 * 
 */
package org.gprom.jdbc.instrumentation;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.log4j.Logger;

/**
 * @author lord_pretzel
 *
 */
public class ConsoleLogger implements IInstrumentationLogger {

	static Logger log = Logger.getLogger(ConsoleLogger.class); 
	
	/* (non-Javadoc)
	 * @see org.grpom.jdbc.instrumentation.IProvenanceLogger#printResult(java.sql.ResultSet, java.lang.String)
	 */
	@Override
	public void printResult(ResultSet rs, String s) throws SQLException {
		int columnCount = rs.getMetaData().getColumnCount();
		StringBuilder buf = new StringBuilder();
		
		for (int i = 1; i <= columnCount; i++) {
			buf.append(rs.getMetaData().getColumnName(i));
			if (i != columnCount)
				buf.append("\t|");
		}
		
		while (rs.next()) {
			for (int i = 1; i <= columnCount; i++)
			{
				buf.append(rs.getString(i) + "\t|");
				if (i != columnCount)
					buf.append("\t|");
			}
			buf.append("\n");
		}
		
		log.info("QUERY RESULT:\n\n" + buf.toString());
	}

	/* (non-Javadoc)
	 * @see org.grpom.jdbc.instrumentation.IQueryLogger#logQuery(java.lang.String)
	 */
	@Override
	public void logQuery(String s) {
		log.info("QUERY:\n" + s);
	}

}
