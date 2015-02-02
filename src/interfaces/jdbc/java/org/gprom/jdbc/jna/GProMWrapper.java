/**
 * 
 */
package org.gprom.jdbc.jna;

import java.sql.SQLException;

/**
 * @author lord_pretzel
 *
 */
public class GProMWrapper implements GProMJavaInterface {

	
	public static GProMWrapper inst = new GProMWrapper ();
	
	public static GProMWrapper getInstance () {
		return inst;
	}
	
	private GProMWrapper () {
		
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#gpromRewriteQuery(java.lang.String)
	 */
	@Override
	public String gpromRewriteQuery(String query) throws SQLException {
		return GProM_JNA.INSTANCE.gprom_rewriteQuery(query);
	}

}
