/**
 * 
 */
package org.gprom.jdbc.metadata_lookup.oracle;

import java.sql.Connection;
import java.sql.SQLException;

import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup;

/**
 * @author lord_pretzel
 *
 */
public class OracleMetadataLookup extends AbstractMetadataLookup {

	public static String[] aggrs = {"max","min","avg","count","sum","first",
		"last","corr","covar_pop","covar_samp","grouping","regr","stddev",
		"stddev_pop","stddev_samp","var_pop","var_samp","variance","xmlagg",
		"stragg"};
	
	/**
	 * @param con
	 * @throws SQLException
	 */
	public OracleMetadataLookup(Connection con) throws SQLException {
		super(con);
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#isWindow(java.lang.String)
	 */
	@Override
	protected int isWindow(String functionName) {
		// TODO Auto-generated method stub
		return 0;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#isAgg(java.lang.String)
	 */
	@Override
	protected int isAgg(String functionName) {
		String fLower = functionName.toLowerCase();
		for(String agg: aggrs)
		{
			if (agg.equals(fLower))
				return 1;
		}
		return 0;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getOpReturnType(java.lang.String, java.lang.String[], int)
	 */
	@Override
	protected String getOpReturnType(String oName, String[] stringArray,
			int numArgs) {
		return stringArray[0]; //TODO check real one
	}

	/**
	 * 
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getViewDefinition(java.lang.String)
	 */
	@Override
	protected String getViewDefinition(String viewName) {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * 
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getTableDef(java.lang.String)
	 */
	@Override
	protected String getTableDef(String tableName) {
		// TODO Auto-generated method stub
		return null;
	}

}
