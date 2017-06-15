/**
 * 
 */
package org.gprom.jdbc.metadata_lookup.sqlite;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.jna.GProMJavaInterface.DataType;
import org.gprom.jdbc.jna.GProMList;
import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup;

/**
 * @author lord_pretzel
 *
 */
public class SQLiteMetadataLookup extends AbstractMetadataLookup {

	private static Logger log = LogManager.getLogger(SQLiteMetadataLookup.class);

	private static Set<String> aggFuncs;
	private static String[] aggFuncList = { "avg", "count", "group_concat", "max", "min", "sum", "total" };
	static {
		aggFuncs = new HashSet<String> ();
		aggFuncs.addAll(Arrays.asList(aggFuncList));
	}
	
	public SQLiteMetadataLookup(Connection con) throws SQLException {
		super(con);
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#isWindow(java.lang.String)
  	 */
	@Override
	public int isWindow(String functionName) {
		return 0;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#isAgg(java.lang.String)
	 */
	@Override
	public int isAgg(String functionName) {
		return aggFuncs.contains(functionName.toLowerCase()) ? 1 : 0;
	}
	
	@Override
	public String getFuncReturnType(String fName, GProMList stringArray,
			int numArgs) {
				
		// aggregation functions
	    if (fName.equalsIgnoreCase("sum")
	            || fName.equalsIgnoreCase( "min")
	            || fName.equalsIgnoreCase( "max")
	        )
	    {
	        DataType argType = DataType.valueOf(stringArray.head.data.ptr_value.getString(0));

	        switch(argType)
	        {
	            case DT_INT:
	            	return DataType.DT_INT.name();
	            case DT_LONG:
	                return DataType.DT_LONG.name();
	            case DT_FLOAT:
	                return DataType.DT_FLOAT.name();
	            default:
	                return DataType.DT_STRING.name();
	        }
	    }
	    
	    if (fName.equalsIgnoreCase("total"))
	    {
	        DataType argType = DataType.valueOf(stringArray.head.data.ptr_value.getString(0));

	        switch(argType)
	        {
	            case DT_INT:
	            	return DataType.DT_INT.name();
	            case DT_LONG:
	            case DT_FLOAT:
	            default:
	                return DataType.DT_FLOAT.name();
	        }
	    }
	    

	    if (fName.equalsIgnoreCase("avg"))
	    {
	    	DataType argType = DataType.valueOf(stringArray.head.data.ptr_value.getString(0));

	        switch(argType)
	        {
	            case DT_INT:
	            	return DataType.DT_INT.name();
	            case DT_LONG:
	            case DT_FLOAT:
	                return DataType.DT_FLOAT.name();
	            default:
	                return DataType.DT_STRING.name();
	        }
	    }

	    if (fName.equalsIgnoreCase("count"))
	        return DataType.DT_INT.name();

	    if (fName.equalsIgnoreCase("group_concat"))
	        return DataType.DT_STRING.name();

	    return DataType.DT_STRING.name();
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getOpReturnType(java.lang.String, java.lang.String[], int)
	 */
	@Override
	public String getOpReturnType(String oName, String[] stringArray, int numArgs) {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getViewDefinition(java.lang.String)
	 */
	@Override
	public String getViewDefinition(String viewName) {
		// TODO Auto-generated method stub
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getTableDef(java.lang.String)
	 */
	@Override
	public String getTableDef(String tableName) {
		// TODO Auto-generated method stub
		return null;
	}

}
