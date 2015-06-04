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

import org.apache.log4j.Logger;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.catalogTableExists_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.catalogViewExists_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.databaseConnectionClose_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.databaseConnectionOpen_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.getAttributeDefaultVal_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.getAttributeNames_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.getAttributes_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.getFuncReturnType_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.getOpReturnType_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.getTableDefinition_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.getViewDefinition_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.isAgg_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.isInitialized_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.isWindowFunction_callback;
import org.gprom.jdbc.utility.LoggerUtil;

import com.sun.jna.StringArray;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

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
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			catch (IllegalAccessException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
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
		plugin.isInitializedPlugin = new isInitialized_callback() {

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
			public void apply(String tableName, PointerByReference attrs,
					IntByReference numArgs) {
				List<String> attrNames = getAttributeNames(tableName);
				
				// wrap resul as string array
				StringArray strArr = new StringArray(attrNames.toArray(new String[] {}));		
				attrs.setPointer(strArr);
				numArgs.setValue(attrNames.size());
			}
		};
		plugin.getAttributes = new getAttributes_callback() {

			@Override
			public void apply(String tableName, PointerByReference attrs,
					PointerByReference dataTypes, IntByReference numArgs) {
				List<String> attrNames = getAttributeNames(tableName);
				List<String> dts = getAttributeDTs(tableName);
				
				StringArray strArr = new StringArray(attrNames.toArray(new String[] {}));		
				attrs.setPointer(strArr);
				StringArray dtArr = new StringArray(dts.toArray(new String[] {}));		
				attrs.setPointer(dtArr);
				
				numArgs.setValue(attrNames.size());
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
			public String apply(String fName, StringArray args, int numArgs) {
				return getFuncReturnType(fName, args.getStringArray(0), numArgs);
			}
			
		};
		plugin.getOpReturnType = new getOpReturnType_callback() {

			@Override
			public String apply(String oName, StringArray args, int numArgs) {
				return getOpReturnType(oName, args.getStringArray(0), numArgs);
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
			// TODO Auto-generated catch block
			e.printStackTrace();
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
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return null;
	}

	/**
	 * @param tableName
	 * @return
	 */
	public List<String> getAttributeDTs(String tableName) {
		List<String> result = new ArrayList<String> ();
		ResultSet rs;
		
		try { //TODO deal correctly with types
			rs = con.getMetaData().getColumns(
				    null, null, tableName, null);
			while(rs.next()){
			    String columnType = sqlTypeToString.get(rs.getInt(5));
			    result.add(columnType);
			}
			rs.close();
		}
		catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return null;
	}

	/**
	 * @param tableName
	 * @return
	 */
	public List<String> getAttributeNames(String tableName) {
		List<String> result = new ArrayList<String> ();
		ResultSet rs;
		
		try {
			rs = con.getMetaData().getColumns(
				    null, null, tableName, null);
			while(rs.next()){
			    String columnName = rs.getString(4);
			    result.add(columnName);
			}
			rs.close();
		}
		catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
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
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
	
}
