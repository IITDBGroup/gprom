package org.gprom.jdbc.driver;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public interface GProMStatementInterface extends Statement {

	/**
	 * Checks if a query has a PERM keywords
	 * 
	 * @param sqlQuery
	 * @return true if the query is a PERM query, false otherwise
	 */
	public boolean checkForGProMKeywords(String sqlQuery);
	
	
	/**
	 * Sends the sql query to the GProM provenance rewriter engine
	 * @param sqlQuery
	 * @return the result set
	 */
	public ResultSet executeGProMQuery(String sqlQuery) throws SQLException;
	
	/**
	 * Gets the database type for using in the PERM module
	 * 1: HSQLDB 2: POSTGRES ...
	 * @return the database type
	 */
	public int getDatabaseType();
}
