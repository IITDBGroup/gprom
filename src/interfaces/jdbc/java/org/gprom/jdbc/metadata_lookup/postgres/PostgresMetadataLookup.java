package org.gprom.jdbc.metadata_lookup.postgres;

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;

import org.apache.log4j.Logger;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.databaseConnectionClose_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.databaseConnectionOpen_callback;
import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup;
import org.gprom.jdbc.utility.LoggerUtil;


public class PostgresMetadataLookup extends AbstractMetadataLookup {
	private static Logger log = Logger.getLogger(PostgresMetadataLookup.class);

	public PostgresMetadataLookup(Connection con) throws SQLException {
		super(con);
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


//	/**
//	 * Checks if the relation name exists and saves also the schema name
//	 * 
//	 * @param name
//	 *            the relation name
//	 * @return the OID of the relation
//	 */
//	public int checkRelationByName(String name, PostgresMaps maps) {
//		ResultSet rs;
//		try {
//			rs = stat.executeQuery("SELECT * FROM pg_class WHERE relname = '"
//					+ name + "'");
//
//			if (rs.next()) {
//				PGClass pgClass = new PGClass();
//				pgClass.setPGClassInfos(rs);
//				maps.hashRelation(pgClass, pgClass.getRelfilenode(), pgClass
//						.getRelName());
//				ResultSet rs2 = null;
//				if(pgClass.getRelkind() == 'r'){
//					rs2 = stat
//						.executeQuery("SELECT schemaname FROM pg_tables WHERE tablename = '"
//								+ name + "'");
//				}else{
//					rs2 = stat
//					.executeQuery("SELECT schemaname FROM pg_views WHERE viewname = '"
//							+ name + "'");
//				}
//				if(rs2.next()){
//					maps.hashName(rs2.getString(1), pgClass.getRelNamespace(),
//							PostgresMaps.SCHEMA_ENDING);
//				}
//				return pgClass.getRelfilenode();
//			} else {
//				return 0;
//			}
//		} catch (SQLException e) {
//			log.error("Error checking for the relation " + name);
//			log.error(e.getMessage());
//		}
//
//		return 0;
//	}
//
//	/**
//	 * Gets the number of attributes of a relation
//	 * 
//	 * @param oid
//	 *            the relation id
//	 * @return
//	 */
//	public int getNumbersOfAttributes(int oid, PostgresMaps maps) {
//		if (maps.hasRelation(oid)) {
//			return maps.getPGClass(oid).getRelnatts();
//		} else {
//			ResultSet rs;
//			try {
//				rs = stat
//						.executeQuery("SELECT relnatts FROM pg_class WHERE relfilenode = "
//								+ oid);
//				if (rs.next()) {
//					return rs.getInt(1);
//				} else {
//					return 0;
//				}
//			} catch (SQLException e) {
//				log
//						.error("Error checking the attribute numbers for relation id "
//								+ oid);
//				log.error(e.getMessage());
//			}
//
//			return 0;
//		}
//	}
//
//	public String getRelationNameByOID(int oid, PostgresMaps maps) {
//		if (maps.hasRelation(oid)) {
//			return maps.getPGClass(oid).getRelName();
//		} else {
//			ResultSet rs;
//			try {
//				rs = stat
//						.executeQuery("SELECT relname FROM pg_class WHERE relfilenode = "
//								+ oid);
//				if (rs.next()) {
//					return rs.getString(1);
//				}
//			} catch (SQLException e) {
//				log.error("Error checking for the relation name by id:" + oid);
//				log.error(e.getMessage());
//			}
//			return null;
//		}
//	}
//
//
//	/**
//	 * Creates for a relation id a PGClass object if it's not already hashed
//	 * 
//	 * @param relationId
//	 *            the relation id
//	 * @param maps
//	 *            the HashMaps
//	 * @return the PGClass object for the relation id
//	 */
//	public PGClass createPGClass(int relationId, PostgresMaps maps) {
//		if (maps.hasRelation(relationId)) {
//			return maps.getPGClass(relationId);
//		} else {
//			ResultSet rs;
//			try {
//				rs = stat
//						.executeQuery("SELECT * FROM pg_class WHERE relfilenode = "
//								+ relationId);
//				if (rs.next()) {
//					PGClass pgClass = new PGClass();
//					pgClass.setPGClassInfos(rs);
//					maps.hashRelation(pgClass, pgClass.getRelfilenode(),
//							pgClass.getRelName());
//					return pgClass;
//				}
//			} catch (SQLException e) {
//				log
//						.error("Error getting the information from the pg_class table for the id: "
//								+ relationId);
//				log.error(e.getMessage());
//			}
//			return null;
//		}
//	}
//
//	/**
//	 * Creates a PGAttribute from the relation
//	 * 
//	 * @param relationId
//	 *            the id of the relation
//	 * @param index
//	 *            the index of the attribute in the relation
//	 * @param maps
//	 *            the HashMaps
//	 * @return FormPGAttribute at position of the index
//	 */
//	public FormPGAttributeClass createFormPGAttribute(int relationId,
//			int index, PostgresMaps maps) {
//		if (maps.hasAttribute(relationId, index)) {
//			return maps.getPGAttribute(relationId, index);
//		} else {
//			try {
//				ResultSet rs = stat
//						.executeQuery("SELECT * FROM pg_attribute WHERE attrelid = "
//								+ relationId + " AND attnum = " + index);
//				if(rs.next()){
//					FormPGAttributeClass pgAttribute = new FormPGAttributeClass();
//					pgAttribute.setPGAttributeInfos(rs);
//					maps.hashAttribute(pgAttribute, index, relationId);
//					return pgAttribute;
//				}
//			} catch (SQLException e) {
//				log.error("Error getting the attribute with index " + index
//						+ " from the relation " + relationId);
//				log.error(e.getMessage());
//			}
//			return null;
//		}
//	}
//
//	
//	/**
//	 * Gets an id for the operator and creates the form pg operator if the operator doesn't
//	 * exists
//	 * @param opername operator name
//	 * @param leftType the left type of the operator
//	 * @param rightType the right type of the operator
//	 * @param maps the HashMaps
//	 * @return id of the operator
//	 */
//
//	public int getOperatorId(String opername, int leftType, int rightType,
//			PostgresMaps maps) {
//		opername = opername.replace('\n', ' ').trim();
//		for (Map.Entry<Integer, FormPGOperator> entry : maps.getOperatorMap()
//				.entrySet()) {
//			int id = entry.getKey();
//			FormPGOperator pgOperator = entry.getValue();
//			if (pgOperator.getOprname().equals(opername)
//					&& pgOperator.getOprleft() == leftType
//					&& pgOperator.getOprright() == rightType) {
//				return id;
//			}
//		}
//
//		try {
//			
//
//			ResultSet rs = stat
//					.executeQuery("SELECT oid,* FROM pg_operator WHERE oprname = '"
//							+ opername + "' AND oprleft = " + leftType
//							+ " AND oprright = " + rightType);
//			if(rs.next()){
//				FormPGOperator pgOperator = new FormPGOperator();
//				pgOperator.setPGOperatorInfos(rs);
//				int id = rs.getInt(1);
//				maps.hashOperator(pgOperator, id, opername);
//				return id;
//			}else{
//				//Search for a suitable operator
//				rs = stat
//				.executeQuery("SELECT oid,* FROM pg_operator WHERE oprname = '"
//						+ opername + "' AND (oprleft IN (" +(leftType-1) + "," + leftType + "," + (leftType + 1) + ") "
//						+ " OR oprright IN (" + (rightType-1) + "," + rightType + "," + (rightType + 1) + "))");
//				if(rs.next()){
//					FormPGOperator pgOperator = new FormPGOperator();
//					pgOperator.setPGOperatorInfos(rs);
//					int id = rs.getInt(1);
//					maps.hashOperator(pgOperator, id, opername);
//					return id;
//				}
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the operator infos for operator "
//					+ opername);
//			log.error(e.getMessage());
//		}
//		return 0;
//	}
//
//	/**
//	 * Create a from pg operator if the operator doesn't exist
//	 * @param oid the id of the operator
//	 * @param opername the operator name
//	 * @param leftType the left type of the operator
//	 * @param rightType the right type of the operator
//	 * @param maps the HashMaps
//	 * @return FormPGOperator
//	 */
//	public FormPGOperator createFormPGOperatorByID(int oid, PostgresMaps maps) {
//		if (maps.hasOperator(oid)) {
//			return maps.getOperator(oid);
//		} else {
//			ResultSet rs;
//			try {
//				rs = stat
//						.executeQuery("SELECT * FROM pg_operator WHERE oid = " + oid);
//				if(rs.next()){
//					FormPGOperator pgOperator = new FormPGOperator();
//					pgOperator.setPGOperatorInfos(rs);
//					int id = maps.getNewIdForOperator();
//					maps.hashOperator(pgOperator, id, pgOperator.getOprname());
//					return pgOperator;
//				}
//			} catch (SQLException e) {
//				log
//						.error("Error getting the operator infos from the pg_operator table");
//				log.error(e.getMessage());
//			}
//			return null;
//		}
//	}
//
//	
//
//	/**
//	 * Checks for a function and return the type
//	 * 
//	 * @param funcName the name of the function
//	 * @param maps the HashMaps
//	 * @return the type of a function 0 = FUNCDETAIL_NOTFOUND; 1 =
//	 *         FUNCDETAIL_MULTIPLE; 2 = FUNCDETAIL_NORMAL; 3 =
//	 *         FUNCDETAIL_AGGREGATE; 4 = FUNCDETAIL_COERCION
//	 */
//	public int checkFunction(String funcName, PostgresMaps maps) {
//		try {
//			ResultSet rs = stat.executeQuery("SELECT proisagg FROM pg_proc WHERE proname = '" + funcName+"'");
//			int id = maps.getNewIdForNameMap();
//			maps.getNameMap().put(funcName, id);
//			maps.getIdMap().put(id, funcName);
//			if(rs.next()){
//				if(rs.getBoolean(1)){
//					return 3;
//				}else{
//					return 2;
//				}
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the function type for the function " + funcName);
//			log.error(e.getMessage());
//		}
//		return 0;
//	}
//	
//	/**
//	 * Check if the identifier exists
//	 * @param identifier the name to check
//	 * @return true the identifier is reserver
//	 */
//	public boolean checkForReservedKeywords(String identifier){
//		try {
//
//			ResultSet rs = stat.executeQuery("SELECT proisagg FROM pg_proc WHERE proname = '" + identifier.toUpperCase()+"'");
//			if(rs.next()){
//				return true;
//			}
//			rs = stat.executeQuery("SELECT proisagg FROM pg_proc WHERE proname = '" + identifier.toLowerCase() +"'");
//			if(rs.next()){
//				return true;
//			}
//		} catch (SQLException e) {
//			log.error("Error checking for a reserved identifier " + identifier);
//			log.error(e.getMessage());
//		}
//		return false;
//	}
//	
//	/**
//	 * Checks if for a type the array type is supported
//	 * @param typeId the type id 
//	 * @return the id of the array type
//	 */
//	public int getArrayTypeId(int typeId, PostgresMaps maps){
//		String typename = maps.getNameByOid(typeId);
//		try {
//			ResultSet rs = stat.executeQuery("SELECT typarray FROM pg_type WHERE typname = '" + typename + "'");
//			if(rs.next()){
//				return rs.getInt(1);
//			}
//		} catch (SQLException e) {
//			log.error("Error getting array type for type " + typename);
//			log.error(e.getMessage());
//		}
//		return 0;
//	}
//	
//	/**
//	 * Creates an equality operator for with the right arguments
//	 * @param leftType the left argument type
//	 * @param rightType the right argument type
//	 * @param maps HashMaps
//	 * @return FormPGOperator for an equality operator
//	 */
//	public FormPGOperator createEqualityOperator(int leftType, int rightType, PostgresMaps maps){
//		//Check if the operator was already hashed
//		if(leftType == 1043){
//			leftType = 1042;
//			rightType = 1042;
//		}
//		
//		for (Map.Entry<Integer, FormPGOperator> entry : maps.getOperatorMap()
//				.entrySet()) {
//			FormPGOperator pgOperator = entry.getValue();
//			if (pgOperator.getOprname().equals("=") && pgOperator.getOprleft() == leftType && pgOperator.getOprright() == rightType){
//				return pgOperator;
//			}
//		}
//		try {
//			ResultSet rs = stat.executeQuery("SELECT * FROM pg_operator WHERE oprname = '=' AND oprleft = " + leftType + " AND oprright = " + rightType);
//			if(rs.next()){
//				FormPGOperator pgOperator = new FormPGOperator();
//				pgOperator.setPGOperatorInfos(rs);
//				int id = maps.getNewIdForOperator();
//				maps.hashOperator(pgOperator, id, "=");
//				return pgOperator;
//			}else{
//				//Search for a suitable equality operator
//				rs = stat.executeQuery("SELECT * FROM pg_operator WHERE oprname = '=' AND (oprleft IN (" +(leftType-1) + "," + leftType + "," + (leftType + 1) + ") "
//						+ " OR oprright IN (" + (rightType-1) + "," + rightType + "," + (rightType + 1) + "))");
//				if(rs.next()){FormPGOperator pgOperator = new FormPGOperator();
//					pgOperator.setPGOperatorInfos(rs);
//					int id = maps.getNewIdForOperator();
//					maps.hashOperator(pgOperator, id, "=");
//					return pgOperator;
//				}
//			}
//		} catch (SQLException e) {
//			log.error("Error getting an equality operator");
//			log.error(e.getMessage());
//		}
//		return null;
//	}
//	
//	public FormPGType createFormPGType(int typeId){
//		
//		//ResultSet rs = stat.executeQuery("SELECT * FROM pg_type WHERE " );
//		return null;
//	}
//	
//	public RewriteRuleClass createRewriteRule(int viewID){
//		
//		try {
//			ResultSet rs = stat.executeQuery("SELECT * FROM pg_rewrite WHERE ev_class = " + viewID);
//			if(rs.next()){
//				RewriteRuleClass rewRule = new RewriteRuleClass();
//				rewRule.setRewriteRuleInfos(rs,0);
//				return rewRule;
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the rewrite rule infos");
//			log.error(e.getMessage());
//		}
//		return null;
//	}
//	
//	/**
//	 * Checks for the schema name if its exists and return an id for the schema
//	 * @param schemaName the schema name to check
//	 * @param maps the HashMaps
//	 * @return id for the map or 0 for not found
//	 */
//	public int checkForSchemaByName(String schemaName, PostgresMaps maps){
//		if(maps.checkForName(schemaName, PostgresMaps.SCHEMA_ENDING)){
//			return maps.getOidByName(schemaName, PostgresMaps.SCHEMA_ENDING);
//		}
//		ResultSet rs;
//		try {
//			rs = stat
//			.executeQuery("SELECT schemaname FROM pg_tables WHERE tablename = "
//					+ schemaName);
//			if(rs.next()){
//				int id = maps.getNewIdForNameMap();
//				maps.hashName(rs.getString(1), id,
//						PostgresMaps.SCHEMA_ENDING);
//				return id;
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the schema name from the pg_tables");
//			log.error(e.getMessage());
//		}
//		return 0;
//	}
//	/**
//	 * Gets for a relation Id the name space id
//	 * @param relationId
//	 * @return the name space id or 0 if not found
//	 */
//	public int getNamespaceIDbyRelationId(int relationId){
//		try {
//			ResultSet rs = stat.executeQuery("SELECT relnamespace FROM pg_class WHERE relfilenode = " + relationId);
//			if(rs.next()){
//				return rs.getInt(1);
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the name space id in the pg_class");
//			log.error(e.getMessage());
//		}
//		return 0;
//	}
//	
//	/**
//	 * Gets the name of the function with the given id
//	 * @param oid id of the function
//	 * @return the name of the function
//	 */
//	public String getFunctionNameByID(int oid, PostgresMaps maps){
//		if(maps.checkForIDInNameMap(oid)){
//			return maps.getNameByOid(oid);
//		}
//		try {
//			ResultSet rs = stat.executeQuery("SELECT proname FROM pg_proc WHERE oid = " +oid);
//			if(rs.next()){
//				return rs.getString(1);
//			}
//		} catch (SQLException e) {
//			log.error("Error getting the function name from the pg_proc table");
//			log.error(e.getMessage());
//		}
//		return null;
//	}
//	
//	/**
//	 * Gets an id for a type
//	 * 
//	 * @param typename
//	 *            the type name
//	 * @param maps
//	 *            the HashMaps
//	 * @return id of a type
//	 */
//	public int getIDForTypeName(String typename, PostgresMaps maps) {
//		if (maps.getNameMap().containsKey(typename)) {
//			return maps.getNameMap().get(typename);
//		} else {
//			try {
//				ResultSet rs = stat.executeQuery("SELECT oid FROM pg_type WHERE typname = '" +typename + "'");
//				if(rs.next()){
//					maps.hashName(typename, rs.getInt(1), "");
//					return rs.getInt(1);
//				}
//			} catch (SQLException e) {
//				log.error("Error getting the type id from type: " + typename);
//				log.error(e.getMessage());
//			}
//			return 25;
//		}
//	}
}
