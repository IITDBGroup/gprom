/**
 * 
 */
package org.gprom.jdbc.jna;

import java.sql.SQLException;

/**
 * @author lord_pretzel
 *
 */
public interface GProMJavaInterface {

	public String gpromRewriteQuery (String query) throws SQLException;
	
}
