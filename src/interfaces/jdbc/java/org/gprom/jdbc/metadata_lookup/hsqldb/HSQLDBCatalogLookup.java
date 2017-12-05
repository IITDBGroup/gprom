package org.gprom.jdbc.metadata_lookup.hsqldb;

import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.SQLException;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup;

public class HSQLDBCatalogLookup extends AbstractMetadataLookup {
	
	private static Logger log = LogManager.getLogger(HSQLDBCatalogLookup.class);
	private Connection con;
	private DatabaseMetaData meta;
	
	public HSQLDBCatalogLookup(Connection con) throws SQLException {
		super(con);
		try {
			meta = this.con.getMetaData();
		} catch (SQLException e) {
			log.error("Error getting the meta data object");
			log.error(e.getMessage());
			System.exit(-1);
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
		// TODO Auto-generated method stub
		return 0;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getOpReturnType(java.lang.String, java.lang.String[], int)
	 */
	@Override
	public String getOpReturnType(String oName, String[] stringArray,
			int numArgs) {
		// TODO Auto-generated method stub
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
	
//	/**
//	 * Checks if the relation name exists and puts the relation in the map with an unique OID
//	 * If a catalog or a schema name exists it will also put in the map
//	 * @param name the relation name
//	 * @return the OID of the relation
//	 */
//	public int checkRelationByName(String name, DBCatalogTopContainer topContainer){
//		try {
//			ResultSet rs = meta.getTables(null, null, name.toUpperCase(), null);
//			if (rs.next()) {
//				//If the relation exists put the schema name and the catalog name in the container
//				ResultSet rsCol = meta.getColumns(null, null, name.toUpperCase(), "%");
//				if (rsCol.next()){
//					//Catalog
//					if (rsCol.getString(1) != null && !topContainer.getUniversalOidMap().containsValue(rsCol.getString(1))){
//						int id = topContainer.getNewOID();
//						topContainer.putObjectInMapByOid(id, rsCol.getString(1));
//					}
//					//Schema
//					if (rsCol.getString(2) != null && !topContainer.getUniversalOidMap().containsValue(rsCol.getString(2))){
//						int id = topContainer.getNewOID();
//						topContainer.putObjectInMapByOid(id, rsCol.getString(2));
//					}
//				}
//				return topContainer.getRelationHolder().putRelationInMap(name.toUpperCase());
//			} else {
//				log.error("The relation doesn't exists");
//				return 0;
//			}
//		} catch (SQLException e) {
//			e.printStackTrace();
//			log.error("Something went wrong while getting the information from the meta data");
//			return 0;
//		}
//	}
//	/**
//	 * Gets the number of attributes of a relation
//	 * @param oid
//	 * @return
//	 */
//	public int getNumbersOfAttributes(int oid, DBCatalogTopContainer topContainer){
//		String relationName = topContainer.getRelationHolder().getRelationByOID(oid);
//		try {
//			ResultSet rs = meta.getColumns(null, null, relationName.toUpperCase(), "%");
//			int i = 0;
//			while (rs.next()){
//				i++;
//			}
//			return i;
//		} catch (SQLException e) {
//			e.printStackTrace();
//			log.error("Error getting the size of the column");
//			return 0;
//		}
//	}
//	
//	/**
//	 * Gets the meta data for the columns for a relation
//	 * @param relName
//	 * @return the ResultSet for the meta data
//	 */
//	public ResultSet getColumnByRelname(String relName){
//		try {
//			return meta.getColumns(null,null,relName.toUpperCase(),"%");
//		} catch (SQLException e) {
//			log.error("Error getting the columns meta data");
//			log.error(e.getMessage());
//		}
//		return null;
//	}
//	
//	/**
//	 * Gets the user name of database
//	 * @return the user name
//	 */
//	public String getUserName(){
//		try {
//			return meta.getUserName();
//		} catch (SQLException e) {
//			log.error("Error getting the user name");
//			log.error(e.getMessage());
//		}
//		return null;
//	}
//	
//	/**
//	 * Check if a primary exists for the relation
//	 * @param relName
//	 * @return true if the relation has a primary key
//	 */
//	public boolean checkForPrimaryKeys(String relName){
//		ResultSet rs = null;
//		try {
//			rs = meta.getPrimaryKeys(null, null, relName);
//			if (rs.next()) {
//				return true;
//			}else{
//				return false;
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the primary key meta data");
//			log.error(e.getMessage());
//			return false;
//		}
//	}
//	
//	/**
//	 * Checks if a relation is a view
//	 * @param relName
//	 * @return true if the relation is a view
//	 */
//	public boolean isView(String relName){
//		ResultSet rs = null;
//		try {
//			rs = meta.getTables(null, null, null,
//					new String[] { "VIEW" });
//			while (rs.next()) {
//				String tableName = rs.getString("TABLE_NAME");
//				if (tableName.equals(relName)) {
//					return true;
//				}
//			}
//			return false;
//		} catch (SQLException e) {
//			log.error("Error checking for a view or a relation");
//			log.error(e.getMessage());
//			return false;
//		}
//
//	}
//
//	/**
//	 * Check if the function exists an returns its type
//	 * 0 = FUNCDETAIL_NOTFOUND 1 = FUNCDETAIL_MULTIPLE 2 = FUNCDETAIL_NORMAL 3 = FUNCDETAIL_AGGREGATE 4 = FUNCDETAIL_COERCION
//	 * @param funcName
//	 * @return
//	 */
//	public int checkFunctionType(String funcName){
//		try {
//			if (meta.getStringFunctions()
//					.contains(funcName.toUpperCase())
//					|| meta.getNumericFunctions()
//							.contains(funcName.toUpperCase())
//					|| meta.getTimeDateFunctions()
//							.contains(funcName.toUpperCase())) {
//				return 2;
//			} else {
//				return 0;
//			}
//		} catch (SQLException e) {
//			log.error("Error checking the function " + funcName);
//			log.error(e.getMessage());
//			return 0;
//		}
//	}
//	/**
//	 * Check if a function name exists
//	 * @param name the function name
//	 * @return true if the function name exists
//	 */
//	public boolean checkForReservedFunctionName(String name, DBCatalogTopContainer topContainer){
//		try {
//			//Check if the name exists
//			ResultSet rs = meta.getProcedures(null, null, "%"+name);
//			if(rs.next()){
//				return true;
//			}
//			//Check if the name exists in Upper Case
//			rs = meta.getProcedures(null, null, "%"+name.toUpperCase());
//			if(rs.next()){
//				return true;
//			}
//		} catch (SQLException e) {
//			log.error("Error checking the procedures");
//			log.error(e.getMessage());
//		}
//		return topContainer.checkForAggregateFunction(name);
//	}
//	
//	/**
//	 * Check if the schema exists an returns the id of the schema
//	 * @param schemaName
//	 * @return the id of the schema or 0 if not exists
//	 */
//	public int checkForSchema(String schemaName, DBCatalogTopContainer topContainer){
//		ResultSet rs;
//		try {
//			rs = meta.getSchemas();
//			while (rs.next()) {
//				if (rs.getString(1).equals(schemaName.toUpperCase())) {
//					return topContainer.putObjectInMap(schemaName
//							.toUpperCase());
//				}
//			}
//		} catch (SQLException e) {
//			log.error("Error checking the schemas");
//			log.error(e.getMessage());
//		}
//		return 0;
//	}
//	
//	public int getNamespaceID(int relationId, DBCatalogTopContainer topContainer){
//		ResultSet rs;
//		try {
//			rs = meta.getColumns(
//					null,
//					null,
//					topContainer.getObjectByOID(relationId)
//							.toUpperCase(), "%");
//			if (rs.next()) {
//				return topContainer.getOIDByName(rs.getString(2));
//			}
//		} catch (SQLException e) {
//			e.printStackTrace();
//			log.error("Error getting the namespace id for a relation id");
//		}
//		return 0;
//	}
//	
//	/**
//	 * Check the type of a relation
//	 * @param name 
//	 * @return "v" for a view, "r" for a relation
//	 */
//	public String checkRelationkind(String name){
//		try {
//			ResultSet rs = meta.getTables(null, null, null, new String[]{"VIEW"});
//			while(rs.next()){
//				if(rs.getString("TABLE_NAME").equals(name.toUpperCase())){
//					return "v";
//				}
//			}
//		} catch (SQLException e) {
//			log.error(e.getMessage());
//			return "r";
//		}
//		return "r";
//	}
//	/**
//	 * Gets all the standard type info from the database
//	 * @return ResultSet meta data for all standard types
//	 */
//	public ResultSet getTypeInfo(){
//		try {
//			return meta.getTypeInfo();
//		} catch (SQLException e) {
//			log.error("Error getting the supported type info of the database");
//			log.error(e.getMessage());
//		}
//		return null;
//	}
//	
//	
//	/**
//	 * Gets a view definition for a view
//	 * @param viewName
//	 * @return the definition of the view
//	 */
//	public String getViewDefinition(String viewName){
//		ResultSet rs;
//		try {
//			rs = con.createStatement().executeQuery("SELECT VIEW_DEFINITION FROM INFORMATION_SCHEMA.SYSTEM_VIEWS WHERE TABLE_NAME = '"+viewName.toUpperCase()+"'");
//			if (rs.next()){
//				return rs.getString(1);
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the view definition from " + viewName);
//			log.error(e.getMessage());
//		}
//		return null;
//	}
//	/**
//	 * Gets for a type id a relation
//	 * @param typeId
//	 * @return the relation type
//	 */
//	public int getRelationByType(int typeId) {
//		return 0;
//	}

}
