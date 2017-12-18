/**
 * 
 */
package org.gprom.jdbc.metadata_lookup.oracle;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.List;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.jna.GProMJavaInterface.DataType;
import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup;

import com.sun.jna.StringArray;

import static org.gprom.jdbc.utility.LoggerUtil.*;

/**
 * @author lord_pretzel
 *
 */
public class OracleMetadataLookup extends AbstractMetadataLookup {

	static Logger log = LogManager.getLogger(OracleMetadataLookup.class);
	
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
	public int isWindow(String functionName) {
		// TODO Auto-generated method stub
		return 0;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#isAgg(java.lang.String)
	 */
	@Override
	public int isAgg(String functionName) {
		String fLower = functionName.toLowerCase();
		for(String agg: aggrs)
		{
			if (agg.equals(fLower))
				return 1;
		}
		return 0;
	}

	@Override
	public String getFuncReturnType(String fName, String[] stringArray,
			int numArgs) {
	    fName = fName.toLowerCase();
		
		// aggregation functions
	    if (fName.equals("sum")
	            || fName.equals( "min")
	            || fName.equals( "max")
	        )
	    {
	        DataType argType = DataType.valueOf(stringArray[0]);

	        switch(argType)
	        {
	            case DT_INT:
	            case DT_LONG:
	                return "DT_LONG";
	            case DT_FLOAT:
	                return "DT_FLOAT";
	            default:
	                return "DT_STRING";
	        }
	    }

	    if (fName.equals("avg"))
	    {
	    	DataType argType = DataType.valueOf(stringArray[0]);

	        switch(argType)
	        {
	            case DT_INT:
	            case DT_LONG:
	            case DT_FLOAT:
	                return "DT_FLOAT";
	            default:
	                return "DT_STRING";
	        }
	    }

	    if (fName.equals("count"))
	        return "DT_LONG";

	    if (fName.equals("xmlagg"))
	        return "DT_STRING";

	    if (fName.equals("row_number"))
	        return "DT_INT";
	    if (fName.equals("dense_rank"))
	        return "DT_INT";

	    if (fName.equals("ceil"))
	    {
	        if(stringArray.length == 1)
	        {
	        	DataType argType = DataType.valueOf(stringArray[0]);
	            switch(argType)
	            {
	                case DT_INT:
	                    return "DT_INT";
	                case DT_LONG:
	                case DT_FLOAT:
	                    return "DT_LONG";
	                default:
	                    ;
	            }
	        }
	    }

	    if (fName.equals("round")) {
	    	 if(stringArray.length == 2) {
	    		 DataType argType = DataType.valueOf(stringArray[0]);
	    		 DataType parType = DataType.valueOf(stringArray[1]);

	            if (parType.equals(DataType.DT_INT)) {
	                switch(argType)
	                {
	                    case DT_INT:
	                        return "DT_INT";
	                    case DT_LONG:
	                    case DT_FLOAT:
	                        return "DT_LONG";
	                    default:
	                        ;
	                }
	            }
	        }
	    }

	    if (fName.equals("greatest") || fName.equals("least")
	            || fName.equals("coalesce") 
	            || fName.equals("first_value") 
	            || fName.equals("last_value")) {
	    		DataType dt = DataType.valueOf(stringArray[0]);

	        for(String g: stringArray) {
	        		DataType argDT = DataType.valueOf(g);
	            dt = lcaType(dt, argDT);
	        }

	        return dt.toString();
	    }
	    
	    if ( fName.equals("lag") || fName.equals("lead")) {
	    		return DataType.valueOf(stringArray[0]).toString();
	    }
	    
	    return "DT_STRING";
	}
	
	
	
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getOpReturnType(java.lang.String, java.lang.String[], int)
	 */
	@Override
	public String getOpReturnType(String oName, String[] stringArray,
			int numArgs) {
		return stringArray[0]; //TODO check real one
	}

	
	/**
	 * 
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getViewDefinition(java.lang.String)
	 */
	@Override
	public String getViewDefinition(String viewName) {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * 
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getTableDef(java.lang.String)
	 */
	@Override
	public String getTableDef(String tableName) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public List<String> getAttributeNames(String tableName) {
		try {
			return super.getAttributeNames(tableName.toUpperCase(), super.con.getSchema());
		}
		catch (SQLException e) {
			logException(e,log);
			return null;
		}
	}
	
	@Override
	public List<String> getAttributeDTs(String tableName) {
		try {
			return super.getAttributeDTs(tableName.toUpperCase(), super.con.getSchema());
		}
		catch (SQLException e) {
			logException(e,log);
			return null;
		}
	}
	
	
	
}
