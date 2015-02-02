package org.gprom.jdbc.container;

import java.util.HashMap;

import org.apache.log4j.Logger;


public class RelationHolder extends DBCatalogTopContainer{

	private static Logger log = Logger.getLogger(RelationHolder.class);
	private HashMap<Integer,String> relationByOIDMap;
	private HashMap<String, Integer> relationByStringMap;
	private DBCatalogTopContainer topHolder;
	
	public RelationHolder(DBCatalogTopContainer topContainer){
		relationByOIDMap = new HashMap<Integer, String>();
		relationByStringMap = new HashMap<String, Integer>();
		topHolder = topContainer;
	}
	
	/**
	 * Gets the relation by the OID
	 * @param oid the OID of a relation
	 * @return the relation name or null if the relation doesn't exist
	 */
	public String getRelationByOID(int oid){
		return relationByOIDMap.get(oid);
	}
	
	/**
	 * Gets the OID of the relation
	 * @param name the relation name
	 * @return the OID if the relation exits or null
	 */
	public Integer getRelationByName(String name){
		return relationByStringMap.get(name);
	}
	
	/**
	 * Puts the relation name in both maps and returns the OID of the relation
	 * @param name the relation name
	 * @param oid the unique id of the relation
	 * @return the OID of the relation
	 */
	public int putRelationInMap(String name){
		if (getRelationByName(name) != null){
			return getRelationByName(name);
		}else{
			int id = getNewOID();
			relationByOIDMap.put(id, name);
			relationByStringMap.put(name,id); 
			topHolder.putObjectInMapByOid(id, name);
			return id;
		}
	}

	public void setDBCatalogObject(DBCatalogTopContainer dbc){
		topHolder = dbc;
	}
}
