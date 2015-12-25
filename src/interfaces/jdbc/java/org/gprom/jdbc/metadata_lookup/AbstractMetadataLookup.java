/**
 * 
 */
package org.gprom.jdbc.metadata_lookup;

import java.lang.reflect.Field;
import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.catalogTableExists_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.catalogViewExists_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.databaseConnectionClose_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.databaseConnectionOpen_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getAttributeDefaultVal_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getAttributeNames_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getDataTypes_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getFuncReturnType_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getKeyInformation_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getOpReturnType_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getTableDefinition_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.getViewDefinition_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.isAgg_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.isInitialized_callback;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin.isWindowFunction_callback;
import org.gprom.jdbc.utility.LoggerUtil;

import com.sun.jna.ptr.PointerByReference;

import static org.gprom.jdbc.utility.LoggerUtil.*;

/**
 * @author lord_pretzel
 *
 */
public abstract class AbstractMetadataLookup {

	private static Logger log = Logger.getLogger(AbstractMetadataLookup.class);

	protected GProMMetadataLookupPlugin plugin;
	protected Connection con;
	private Statement stat;
	public static Map<Integer,String> sqlTypeToString = getAllJdbcTypeNames();
	
	/**
	 * Use reflection to map sql types int constants to strings (stored in java.sql.Types).
	 * 
	 * @return a map<Integer,String) storing the type mapping
	 */
	private static  Map<Integer, String> getAllJdbcTypeNames() {
	    Map<Integer, String> result = new HashMap<Integer, String>();

	    for (Field field : java.sql.Types.class.getFields()) {
	        try {
				result.put((Integer)field.get(null), field.getName());
			}
			catch (IllegalArgumentException e) {
				logException(e,log);
			}
			catch (IllegalAccessException e) {
				logException(e,log);
			}
	    }

	    return result;
	}
	
	/**
	 * Stores information about an SQL function.
	 * 
	 * @author lord_pretzel
	 *
	 */
	public class FunctionDesc {
		int returnType;
		List<Integer> parameters;
		String fName;	
		
		public FunctionDesc () { 
			parameters = new ArrayList<Integer> ();
		}
		
		public String toString() {
			return fName + "(" + parameters.toString() + ") -> " + returnType;
		}
	}
	
	/**
	 * 
	 * @param con
	 * @throws SQLException
	 */
	public AbstractMetadataLookup(Connection con) throws SQLException {
		this.con = con;
		createPlugin(this);
	}
	
	public GProMMetadataLookupPlugin getPlugin() {
		return plugin;
	}

	/**
	 * Creates a plugin. The plugin structure is fixed and all methods are deligated to the methods defined in this class. 
	 */
	private void createPlugin(final AbstractMetadataLookup t) {
		plugin = new GProMMetadataLookupPlugin ();
		plugin.isInitialized = new isInitialized_callback() {

			@Override
			public int apply() {
				return 1;
			}
			
		};
		plugin.databaseConnectionClose= new databaseConnectionClose_callback() {

			@Override
			public int apply() {
				return t.closeConnection();
			}
			
		};
		plugin.databaseConnectionOpen = new databaseConnectionOpen_callback() {

			@Override
			public int apply() {
				return t.openConnection();
			}
			
		};
		plugin.catalogTableExists = new catalogTableExists_callback() {

			@Override
			public int apply(String tableName) {
				return tableExists(tableName);
			}
			
		};
		plugin.catalogViewExists = new catalogViewExists_callback() {

			@Override
			public int apply(String viewName) {
				return viewExists(viewName);
			}
			
		};
		plugin.getAttributeDefaultVal = new getAttributeDefaultVal_callback() {

			@Override
			public String apply(String schema, String tableName,
					String attrName) {
				return getAttrDefValue(schema,tableName,attrName);
			}
			
		};
		plugin.getAttributeNames = new getAttributeNames_callback() {

			@Override
			public String apply(String tableName) {
				List<String> attrNames = getAttributeNames(tableName);
				String result = null;
				
				result = listToString(attrNames);
				
				log.debug("attrs for table " + tableName + " are " + attrNames);
				
				return result;
			}
		};
		plugin.getDataTypes = new getDataTypes_callback() {

			@Override
			public String apply(String tableName) {
				List<String> dts = getAttributeDTs(tableName);
				String result = null;
				
				result = listToString(dts);
				
				log.debug("dts for table " + tableName + " are " + dts);
				
				return result;
			}
			
		};
		plugin.isAgg = new isAgg_callback() {

			@Override
			public int apply(String functionName) {
				return isAgg(functionName);
			}
			
		};
		plugin.isWindowFunction = new isWindowFunction_callback() {

			@Override
			public int apply(String functionName) {
				return isWindow(functionName);
			}
		};
		plugin.getFuncReturnType = new getFuncReturnType_callback() {

			@Override
			public String apply(String fName, PointerByReference args,
					int numArgs) {
				String[] fArgs;
				fArgs = args.getPointer().getStringArray(0, numArgs);
				return getFuncReturnType(fName, fArgs, numArgs);
			}
			
		};
		plugin.getOpReturnType = new getOpReturnType_callback() {

			@Override
			public String apply(String oName, PointerByReference args,
					int numArgs) {
				String[] opArgs;
				opArgs = args.getPointer().getStringArray(0, numArgs);
				return getOpReturnType(oName, opArgs, numArgs);
			}
			
		};
		plugin.getTableDefinition = new getTableDefinition_callback() {

			@Override
			public String apply(String tableName) {
				return getTableDef(tableName);
			}
			
		};
		plugin.getViewDefinition = new getViewDefinition_callback() {

			@Override
			public String apply(String viewName) {
				return getViewDefinition(viewName);
			}
			
		};
		plugin.getKeyInformation = new getKeyInformation_callback() {

			@Override
			public String apply(String tableName) {
				try {
					List<String> key = getKeyInformation(tableName); 
					return listToString(key);
				}
				catch (SQLException e) {
					logException(e, log);
				}
				return null;
			}
			
		};
	}

	/**
	 * @param tableName
	 * @return
	 * @throws SQLException 
	 */
	public List<String> getKeyInformation(String tableName) throws SQLException {
		return getKeyInformation(tableName, null);
	}

	protected List<String> getKeyInformation(String tableName, String schema) throws SQLException {
		ResultSet rs;
		List<String> result = new ArrayList<String> ();

		rs = con.getMetaData().getPrimaryKeys(null, schema, tableName);

	    while (rs.next()) {
	    	String columnName = rs.getString("COLUMN_NAME");
		    result.add(columnName);
		}
	    
	    log.debug("keys for relation " + tableName + " are " + result);
	    
		return result;
	}

	
	/**
	 * @param viewName
	 * @return
	 */
	public int viewExists(String viewName) {
		return tableExistsForTypes(viewName, new String[] {"VIEW"}) ? 1 : 0;	}

	/**
	 * @param tableName
	 * @return
	 */
	public int tableExists(String tableName) {
		return tableExistsForTypes(tableName, new String[] {"TABLE"}) ? 1 : 0;
	}
	
	public boolean tableExistsForTypes (String tableName, String[] types) {
		ResultSet rs;
		boolean exists = false;
		try {
			rs = con.getMetaData().getTables(null,
	                  null,
	                  tableName,
	                  types);
			while(rs.next()){
			    exists = true;
			}
			rs.close();
		}
		catch (SQLException e) {
			logException(e,log);
		}
		
		return exists;
	}

	/**
	 * @param functionName
	 * @return
	 */
	public abstract int isWindow(String functionName);
	/**
	 * @param functionName
	 * @return
	 */
	public abstract int isAgg(String functionName);

	/**
	 * @param oName
	 * @param stringArray
	 * @param numArgs
	 * @return
	 */
	public abstract String getOpReturnType(String oName, String[] stringArray,
			int numArgs);
	/**
	 * @param viewName
	 * @return the SQL defining viewName
	 */
	public abstract String getViewDefinition(String viewName);

	/**
	 * @param tableName
	 * @return 
	 */
	public abstract String getTableDef(String tableName);

	/**
	 * @param fName
	 * @param stringArray
	 * @param numArgs
	 * @return
	 */
	public String getFuncReturnType(String fName, String[] stringArray,
			int numArgs) {
		ResultSet rs;
		FunctionDesc f = null;
		List<FunctionDesc> fs = new ArrayList<FunctionDesc> ();
		
		try { 
			//TODO deal with types correctly
			rs = con.getMetaData().getFunctionColumns(
				    null, null, fName, null);
			while(rs.next()) {
				String funcName = rs.getString("FUNCTION_NAME");
			    short columnType = rs.getShort("COLUMN_TYPE");
			    int dt = rs.getInt("DATA_TYPE");
			    int pos = rs.getInt("ORDINAL_POSITION");
			    
			    // found new function
			    if (f == null || ! f.fName.equals(f.fName)) {
					f = new FunctionDesc();
					fs.add(f);
					f.fName = funcName;
				}
			    if (columnType == DatabaseMetaData.functionColumnIn) {
			    	f.parameters.set(pos, dt);
			    }
			    if (columnType == DatabaseMetaData.functionColumnResult) {
			    	f.returnType = dt;
			    }
			}
			if (fs.size() > 1)
				throw new SQLException("ambigious function name " + fName + "\n" + fs.toString());
			if (fs.size() == 0)
				throw new SQLException("function does not exist " + fName);
			rs.close();
			return sqlTypeToString.get(f.returnType);
		}
		catch (SQLException e) {
			logException(e,log);
		}
		
		return null;
	}

	/**
	 * @param tableName
	 * @return
	 */
	public List<String> getAttributeDTs(String tableName) {
		return getAttributeDTs(tableName, null);
	}

	protected List<String> getAttributeDTs(String tableName, String schema) {
		List<String> result = new ArrayList<String> ();
		ResultSet rs;
		
		try { //TODO deal correctly with types
			rs = con.getMetaData().getColumns(
				    null, schema, tableName, null);
			while(rs.next()){
			    String columnType = sqlTypeToString.get(rs.getInt(5));
			    String dt = sqlToGpromDT(columnType);
			    result.add(dt);
			}
			rs.close();
			
			return result;
		}
		catch (SQLException e) {
			logException(e,log);
		}
		
		return null;
	}
	
	/**
	 * @param tableName
	 * @return
	 */
	public List<String> getAttributeNames(String tableName) {
		return getAttributeNames(tableName,null);
	}
	
	protected List<String> getAttributeNames(String tableName, String schema) {
		List<String> result = new ArrayList<String> ();
		ResultSet rs;
		
		try {
			rs = con.getMetaData().getColumns(
				    null, schema, tableName, null);
			while(rs.next()){
			    String columnName = rs.getString(4);
			    result.add(columnName);
			}
			rs.close();
			
			return result;
		}
		catch (SQLException e) {
			logException(e, log);
		}
		
		return null;
	}
	
	/**
	 * 
	 * @return
	 */
	public int openConnection () {
		try {
			stat = con.createStatement();
			return 0;
		}
		catch (SQLException e) {
			LoggerUtil.logException(e, log);
			return 1;
		}
	}
	
	/**
	 * 
	 * @return
	 */
	public int closeConnection () {
		try {
			stat.close();
			return 0;
		}
		catch (SQLException e) {
			LoggerUtil.logException(e, log);
			return 1;
		}
	}
	
	/**
	 * 
	 * @param schema
	 * @param tableName
	 * @param attrName
	 * @return
	 */
	public String getAttrDefValue (String schema, String tableName, String attrName)
	{
		ResultSet rs;
		try {
			rs = con.getMetaData().getColumns(
				    null, null, tableName, attrName);
			while(rs.next()){
			    String columnName = rs.getString("COLUMN_DEF");
			    return columnName;
			}
			rs.close();
		}
		catch (SQLException e) {
			logException(e,log);
		}
		return null;
	}
	
	protected String sqlToGpromDT(String dt) {
		if (dt.equals("VARCHAR") || dt.equals("VARCHAR2")) {
			return "DT_STRING";
		}
		if (dt.equals("INT")) {
			return "DT_INT";
		}
		if (dt.equals("DECIMAL")) {
			return "DT_INT";
		}
		//TODO
		return "DT_STRING";
	}
	
	protected String listToString (List<String> in) {
		String result = null;
		
		for(String s: in)
			if (result == null)
				result = s;
			else
				result += "," + s;
		
		return result;
	}
}
