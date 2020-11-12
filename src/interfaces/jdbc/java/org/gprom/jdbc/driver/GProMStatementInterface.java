package org.gprom.jdbc.driver;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import org.gprom.jdbc.driver.GProMJDBCUtil.BackendType;

public interface GProMStatementInterface extends Statement {

	/**
	 * Checks if a query has a provenance related keywords
	 * 
	 * @param sqlQuery
	 * @return true if the query is a query that needs provenance rewrite query, false otherwise
	 */
	public boolean checkForGProMKeywords(String sqlQuery);
	
	
	/**
	 * Sends the sql query to the GProM provenance rewriter engine
	 * @param sqlQuery
	 * @return the result set
	 */
	public ResultSet executeGProMQuery(String sqlQuery) throws SQLException;
	
	/**
	 * Gets the backend database type for this connection 
	 * @return the database type
	 */
	public BackendType getDatabaseType();
}
