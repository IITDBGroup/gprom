/**
 * 
 */
package org.gprom.jdbc.metadata_lookup.oracle;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Formatter;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.jna.GProMJavaInterface.DataType;
import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup;
import org.gprom.jdbc.utility.LoggerUtil;

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
	
	private static final String SQLgetAttrNames = "SELECT COLUMN_NAME, DATA_TYPE\n" + 
			"  FROM SYS.DBA_TAB_COLS_V$ \n" + 
			"  WHERE TABLE_NAME = ?\n" + 
			"  AND OWNER = ?";	
	private static final String SQLgetAttrDefault = "SELECT DATA_DEFAULT\n" + 
			"  FROM SYS.DBA_TAB_COLS_V$ \n" + 
			"  WHERE TABLE_NAME = ?\n" + 
			"  AND OWNER = ?\n" +
			"  AND COLUMN_NAME = ?";	
	private static final String SQLgetPKs = "SELECT cols.column_name \n" + 
			" FROM all_constraints cons, all_cons_columns cols \n" + 
			" WHERE cols.table_name = ? \n" + 
			" AND cons.owner = ? \n" +
			" AND cons.constraint_type = 'P' \n" + 
			" AND cons.constraint_name = cols.constraint_name \n" + 
			" AND cons.owner = cols.owner \n" + 
			" ORDER BY cols.table_name, cols.position";
	private static final String SQLtableExists = "SELECT count(*) FROM SYS.dba_tables "
			+ "WHERE TABLE_NAME = ? AND OWNER = ?";
	private static final String SQLviewExists = "SELECT count(*) FROM dba_views "
			+ "WHERE VIEW_NAME = ? AND OWNER = ?";
	private static final String SQLExplainPlan = "EXPLAIN PLAN FOR %s;";
	private static final String SQLGetCost = "SELECT MAX(COST) FROM PLAN_TABLE;";
	private static final String SQLCleanPlanTable = "DELETE FROM PLAN_TABLE;";
	
	private enum StmtTypes {
		getAttr,
		getAttrDef,
		getPKs,
		tableExists,
		viewExists
	}
	
	private Map<StmtTypes,PreparedStatement> prepStmts;
	
	/**
	 * @param con
	 * @throws SQLException
	 */
	public OracleMetadataLookup(Connection con) throws SQLException {
		this.con = con;
		createPlugin(this);
		setupStmts();
	}
	
	private void setupStmts () throws SQLException {
		prepStmts = new HashMap<StmtTypes,PreparedStatement> ();
		createPreparedStmt(StmtTypes.getAttr, SQLgetAttrNames);
		createPreparedStmt(StmtTypes.getAttrDef, SQLgetAttrDefault);
		createPreparedStmt(StmtTypes.getPKs, SQLgetPKs);
		createPreparedStmt(StmtTypes.tableExists, SQLtableExists);
		createPreparedStmt(StmtTypes.viewExists, SQLviewExists);
	}
	
	private void createPreparedStmt (StmtTypes key, String stmt) throws SQLException {
		PreparedStatement p;
		p = con.prepareStatement(stmt);
		prepStmts.put(key, p);
	}

	private ResultSet runPrepStmt (StmtTypes typ, String ... params) throws SQLException {
		ResultSet rs = null;
		PreparedStatement p;
		int i = 1;
		
		p = prepStmts.get(typ);
		for(String par: params) {
			p.setString(i++, par);
		}
		rs = p.executeQuery();
		
		return rs;
	}
	
	private ResultSet runQuery (Statement s, String query) throws SQLException {
		ResultSet rs = null;
		
		rs =  s.executeQuery(query);
		
		return rs;
	}
	
	private void safeClose(ResultSet rs) {
		if (rs != null) {
			try { 
				rs.close();
			} catch (Exception e2) {
				
			}
		}
	}
	
	/**
	 * 
	 * @return
	 */
	public int closeConnection () {
		try {
			closePreparedStmts();
			return 0;
		}
		catch (SQLException e) {
			LoggerUtil.logException(e, log);
			return 1;
		}
	}
		
	private void closePreparedStmts() throws SQLException {
		for(PreparedStatement p: prepStmts.values()) {
			if( p != null)
				p.close();
		}
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
	protected List<String> getKeyInformation(String tableName, String schema) throws SQLException {
		List<String> result = new ArrayList<String> ();
		ResultSet rs = null;
		
		try {
			rs = runPrepStmt(StmtTypes.getPKs, tableName, con.getSchema());
			while(rs.next()){
			    String columnName = rs.getString(1);
			    result.add(columnName);
			}
			rs.close();
			
			return result;
		}
		catch (SQLException e) {
			logException(e, log);
		}
		finally {
			safeClose(rs);
		}
		
		return null;
	}
	
	/**
	 * @param viewName
	 * @return
	 */
	public int viewExists(String viewName) {
		boolean result = tableExistsForTypes(viewName, false);
		log.debug("table {} exists {}", viewName, result);	
		return result ? 1 : 0;
	}
	
	/**
	 * @param tableName
	 * @return
	 */
	public int tableExists(String tableName) {
//		return 1;
		boolean result = tableExistsForTypes(tableName, true); 
		log.debug("table {} exists {}", tableName, result);
		return result ? 1 : 0;
	}
	
	public boolean tableExistsForTypes (String tableName, boolean isTable) {
		int result = -1;
		ResultSet rs = null;
		
		log.debug("check for tablename {} is table? {}", tableName, isTable);
		
		try {
			if (isTable)
				rs = runPrepStmt(StmtTypes.tableExists, tableName, con.getSchema());
			else // view
				rs = runPrepStmt(StmtTypes.viewExists, tableName, con.getSchema());
			while(rs.next()){
			    result = rs.getInt(1);
			}
		    rs.close();
			
		    log.debug("table {} exists: {}", tableName, result);
		    
			return result == 1;
		}
		catch (SQLException e) {
			logException(e, log);
		}
		finally {
			safeClose(rs);
		}
		
		return false;
	}
	
	/**
	 * 
	 * @param schema
	 * @param tableName
	 * @param attrName
	 * @return
	 */
	@Override
	public String getAttrDefValue (String schema, String tableName, String attrName)
	{
		String result = null;
		ResultSet rs = null;
		
		try {
			rs = runPrepStmt(StmtTypes.getAttrDef, tableName, con.getSchema(), attrName);
			while(rs.next()){
			    String columnName = rs.getString(1);
			    rs.close();
			    result = columnName;
			}
			
			return result;
		}
		catch (SQLException e) {
			logException(e, log);
		}
		finally {
			safeClose(rs);
		}
		
		return null;
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
	protected List<String> getAttributeNames(String tableName, String schema) {
		List<String> result = new ArrayList<String> ();
		ResultSet rs = null;
		
		try {
			rs = runPrepStmt(StmtTypes.getAttr, tableName, con.getSchema());
			while(rs.next()){
			    String columnName = rs.getString(1);
			    result.add(columnName);
			}
			rs.close();
			
			return result;
		}
		catch (SQLException e) {
			logException(e, log);
		}
		finally {
			safeClose(rs);
		}
		
		return null;
	}
	
	@Override
	protected List<String> getAttributeDTs(String tableName, String schema) {
		List<String> result = new ArrayList<String> ();
		ResultSet rs = null;
		
		try {
			rs = runPrepStmt(StmtTypes.getAttr, tableName, con.getSchema());
			while(rs.next()){
			    String columnType = rs.getString(2);
			    String dt = sqlToGpromDT(columnType);
			    result.add(dt);
			}
			rs.close();
			
			return result;
		}
		catch (SQLException e) {
			logException(e, log);
		}
		finally {
			safeClose(rs);
		}
		
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
	
	@Override
	public int getCostEstimation (String query) {
		Statement s; 
		String oneLineQ = query.replace("\n", " ");
		StringBuilder explain = new StringBuilder();
		ResultSet rs = null;
		int cost = -1;
		Formatter f = new Formatter(explain);
		
		try {
			s = con.createStatement();
			f.format(SQLExplainPlan, oneLineQ);
			rs = runQuery(s, explain.toString());
			safeClose(rs);
			
			rs = runQuery(s, SQLGetCost);
			if (rs.next()) {
				cost = rs.getInt(1);
			}
			safeClose(rs);
			
			s.execute(SQLCleanPlanTable);
			s.close();
		}
		catch (SQLException e) {
			LoggerUtil.logException(e, log);
		}
		finally {
			f.close();
		}
		return cost;
	}
	
	
}
