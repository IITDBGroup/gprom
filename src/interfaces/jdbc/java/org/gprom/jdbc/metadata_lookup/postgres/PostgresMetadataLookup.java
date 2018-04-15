package org.gprom.jdbc.metadata_lookup.postgres;

import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.jna.GProMJavaInterface.DataType;
import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup;
import org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup.FunctionDesc;

import static org.gprom.jdbc.jna.GProMJavaInterface.DataType.*;
import static org.gprom.jdbc.utility.LoggerUtil.logException;

public class PostgresMetadataLookup extends AbstractMetadataLookup {
	
	private static Logger log = LogManager.getLogger(PostgresMetadataLookup.class);

	enum MetadataStmtType {
		GetFuncReturnType,
		isAggFunction,
		GetOpReturnType,
		GetPK,
		isWinFunc,
		GetViewDef,
		OidGetTypename
	}
	
	private Map<MetadataStmtType, PreparedStatement> stmts;
	private Map<String, String> viewDefs;
	private HashMap<String, Boolean> aggFuncs;
	private HashMap<String, Boolean> winFuncs;
	private Map<FuncDef, DataType> funcReturnTypes;
	private Map<FuncDef, DataType> opReturnTypes;
	private Map<Integer, String> oidToTypename;
	
	private static class FuncDef {
		private final DataType[] inTypes;
		private final String name;
		
		public FuncDef (String name, DataType[] inTypes) {
			this.name = name;
			this.inTypes = inTypes;
		}
		
		@Override
		public boolean equals(Object o) {
			FuncDef other;
			if (! (o instanceof FuncDef))
				return false;
			
			other = (FuncDef) o;
			
			if(!this.name.equals(other.name))
				return false;
			
			if (!Arrays.equals(this.inTypes, other.inTypes))
				return false;
			
			return true;
		}
		
		@Override
		public int hashCode () {
			int result = name.hashCode();
			result |= Arrays.hashCode(inTypes);
			
			return result;
		}
		
	}
	
	public PostgresMetadataLookup(Connection con) throws SQLException {
		super(con);
		stmts = new HashMap<MetadataStmtType, PreparedStatement> ();
		prepareStatements();
		setupMetadataCache();
	}

	/**
	 * 
	 */
	private void setupMetadataCache() {
		viewDefs = new HashMap<String, String> ();
		aggFuncs = new HashMap<String, Boolean> ();
		winFuncs = new HashMap<String, Boolean> ();
		funcReturnTypes = new HashMap<FuncDef, DataType> ();
		opReturnTypes = new HashMap<FuncDef, DataType> ();
		oidToTypename = new HashMap<Integer, String> ();
	}

	/**
	 * @throws SQLException 
	 * 
	 */
	private void prepareStatements() throws SQLException {
		stmts.put(MetadataStmtType.isAggFunction, 
				con.prepareStatement("SELECT bool_or(proisagg) AS is_agg FROM pg_proc WHERE proname = ?::text;"));
		stmts.put(MetadataStmtType.GetFuncReturnType, 
				con.prepareStatement("SELECT prorettype, proargtypes, proallargtypes FROM pg_proc WHERE proname = ?::name AND pronargs = ?::smallint;"));		
		stmts.put(MetadataStmtType.GetOpReturnType, 
				con.prepareStatement("SELECT oprleft, oprright, oprresult FROM pg_operator WHERE oprname = ?::name;"));		
		stmts.put(MetadataStmtType.GetPK, 
				con.prepareStatement("SELECT a.attname FROM pg_constraint c, pg_class t, pg_attribute a WHERE c.contype = 'p' AND c.conrelid = t.oid AND t.relname = ?::text AND a.attrelid = t.oid AND a.attnum = ANY(c.conkey);"));
		stmts.put(MetadataStmtType.isWinFunc, 
				con.prepareStatement("SELECT bool_or(proiswindow OR proisagg) is_win FROM pg_proc WHERE proname = ?::text;"));
		stmts.put(MetadataStmtType.GetViewDef, 
				con.prepareStatement("SELECT definition FROM pg_views WHERE viewname = ?::text;"));
		stmts.put(MetadataStmtType.OidGetTypename, 
				con.prepareStatement("SELECT typname FROM pg_type WHERE oid = ?::oid;"));
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getOpReturnType(java.lang.String, java.lang.String[], int)
	 */
	@Override
	public String getOpReturnType(String oName, String[] stringArray,
			int numArgs)  {
		DataType[] args = Arrays.stream(stringArray).map(x -> DataType.valueOf(x)).toArray(DataType[]::new);
		FuncDef oIn= new FuncDef(oName, args);
		if (opReturnTypes.containsKey(oIn))
			return opReturnTypes.get(oIn).toString();
		
		try {
			PreparedStatement s = stmts.get(MetadataStmtType.GetOpReturnType);
			s.setString(1, oName);
			ResultSet rs = s.executeQuery();
			DataType[] inTypes = new DataType[2];
			
			while(rs.next()) {
				String retTypeString = rs.getString(3); 
				String lOidString = rs.getString(1);
				String rOidString = rs.getString(2);
				inTypes[0] = oidToDT(lOidString);
				inTypes[1] = oidToDT(rOidString);
				DataType retType = oidToDT(retTypeString);
				
				opReturnTypes.put(new FuncDef (oName, inTypes), retType);
				
				if (Arrays.equals(inTypes, args)) {
					return retType.toString();
				}
			}
			
			rs.close();
		
		} catch (SQLException e) {
			log.error("error while trying to determine op return type: ", e);
		}
	
		return DT_STRING.toString();
	}

	@Override
	public String getFuncReturnType(String fName, String[] stringArray,
			int numArgs) {
		DataType[] args = Arrays.stream(stringArray).map(x -> DataType.valueOf(x)).toArray(DataType[]::new);
		FuncDef fIn= new FuncDef(fName, args);
		if (funcReturnTypes.containsKey(fIn))
			return funcReturnTypes.get(fIn).toString();
		
		try {
			PreparedStatement s = stmts.get(MetadataStmtType.GetFuncReturnType);
			s.setString(1, fName);
			s.setInt(2, numArgs);
			
			ResultSet rs = s.executeQuery();
			
			while(rs.next()) {
				String retTypeString = rs.getString(1); 
				String oidString = rs.getString(2);
				DataType[] inTypes = oidVecToDataTypes(oidString);
				DataType retType = oidToDT(retTypeString);
				
				funcReturnTypes.put(new FuncDef (fName, inTypes), retType);
				
				if (Arrays.equals(inTypes, args)) {
					return retType.toString();
				}
			}
			
			rs.close();
		
		} catch (SQLException e) {
			log.error("error while trying to determine op return type: ", e);
		}
	
		return DT_STRING.toString();
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#getViewDefinition(java.lang.String)
	 */
	@Override
	public String getViewDefinition(String viewName) {
		String result = null;
		
		if (viewDefs.containsKey(viewName))
			return viewDefs.get(viewName);
		
		try {
			PreparedStatement s = stmts.get(MetadataStmtType.GetViewDef);
			s.setString(1, viewName);
			ResultSet rs = s.executeQuery();

			
			while(rs.next()) {
				result = rs.getString(1).trim(); 
			}
			
			if (result != null)
				viewDefs.put(viewName, result);
			
			rs.close();
		
		} catch (SQLException e) {
			log.error("error while trying to determine definition of view: ", e);
		}
		
		return result;
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
		if (winFuncs.containsKey(functionName))
			return winFuncs.get(functionName) ? 1 : 0;
		
		try {
			PreparedStatement s = stmts.get(MetadataStmtType.isWinFunc);
			s.setString(1, functionName);
			ResultSet rs = s.executeQuery();
			String retString = null;
			boolean isWin;
			
			while(rs.next()) {
				retString = rs.getString(1); 
			}
						
			rs.close();
			
			isWin = ("t".equals(retString));
			winFuncs.put(functionName, isWin);
			
			return (isWin) ? 1 : 0;		
		} catch (SQLException e) {
			log.error("error while trying to determine whether function is aggregation function", e);
		}
	
		return 0;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.metadata_lookup.AbstractMetadataLookup#isAgg(java.lang.String)
	 */
	@Override
	public int isAgg(String functionName) {
		if (aggFuncs.containsKey(functionName))
			return aggFuncs.get(functionName) ? 1 : 0;
		
		try {
			PreparedStatement s = stmts.get(MetadataStmtType.isAggFunction);
			s.setString(1, functionName);
			ResultSet rs = s.executeQuery();
			String retString = null;
			boolean isAgg;
			
			while(rs.next()) {
				retString = rs.getString(1); 
			}
			
			rs.close();
			
			isAgg = ("t".equals(retString));
			aggFuncs.put(functionName, isAgg);
			
			return (isAgg) ? 1 : 0;		
		} catch (SQLException e) {
			log.error("error while trying to determine whether function is aggregation function", e);
		}
	
		return 0;
	}

	private DataType oidToDT (String oid) {
		return postgresTypenameToDT(oidToTypeName(oid));
	}
	
	private String oidToTypeName (String oid) {
		String typName = null;
		int oidInt = Integer.parseInt(oid);
		
		if (oidToTypename.containsKey(oidInt))
			oidToTypename.get(oidInt);
		
		try {
			PreparedStatement s = stmts.get(MetadataStmtType.OidGetTypename);
			s.setInt(1, oidInt);
			ResultSet rs = s.executeQuery();
			
			while(rs.next()) {
				typName = rs.getString(1); 
			}
			
			rs.close();
			
			oidToTypename.put(oidInt, typName);
				
		} catch (SQLException e) {
			log.error("error while trying to determine whether function is aggregation function", e);
		}
		
		return typName;
	}
	
	private DataType[] oidVecToDataTypes (String oidString) {
		String[] oids = oidString.split("\\s+");
		DataType[] result = new DataType[oids.length];
		
		for(int i = 0; i < result.length; i++) {
			result[i] = oidToDT(oids[i]);
		}
		
		return result;
	}
	
	private DataType postgresTypenameToDT (String typName) {
		if (typName.equals("char")
				|| typName.equals("name")
				|| typName.equals("text")
				|| typName.equals("tsquery")
				|| typName.equals("varchar")
				|| typName.equals("xml")
				)
			return DT_STRING;

		// integer data types
		if (typName.equals("int2")
				|| typName.equals("int4"))
			return DT_INT;

		// long data types
		if (typName.equals("int8"))
			return DT_LONG;

		// numeric data types
		if (typName.equals("float4")
				|| typName.equals("float8")
				)
			return DT_FLOAT;

		// boolean
		if (typName.equals("bool"))
			return DT_BOOL;

		return DT_STRING;
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
