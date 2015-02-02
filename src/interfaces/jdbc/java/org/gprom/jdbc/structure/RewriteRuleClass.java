package org.gprom.jdbc.structure;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.log4j.Logger;
import org.gprom.jdbc.container.DBCatalogTopContainer;
import org.gprom.jdbc.metadata_lookup.hsqldb.HSQLDBCatalogLookup;

/**
 * Class for the RewriteRule struct
 * 
 * @author alex
 * 
 */
public class RewriteRuleClass {
	
	private static Logger log = Logger.getLogger(RewriteRuleClass.class);

	int ruleId;
	/*	
	 * 1= CMD_SELECT, 2= CMD_UPDATE, 3 = CMD_INSERT, 4 = CMD_DELETE, 5 = CMD_UNKNOWN
	 */
	int event;
	
	int attrno;
	
	String actions;
	
	char enabled;
	
	boolean isInstead;
	
	/*
	 * 0 if the rewrite rule is already transformed, 1 if the view definition has to be transformed
	 */
	int rewriteRule;
	public RewriteRuleClass(int viewId, HSQLDBCatalogLookup lookUp, DBCatalogTopContainer topContainer){
		String viewName = topContainer.getObjectByOID(viewId);
		actions = lookUp.getViewDefinition(viewName);
		event = 1;
		enabled = 'O';
		attrno = -1;
		isInstead = true;
		rewriteRule = 1;
		ruleId = topContainer.putObjectInMap(viewId+"_rule");
	}

	public RewriteRuleClass() {
	}

	/**
	 * Sets all the values from the pg_rewrite table to the RewriteRuleClass fields
	 * @param rs the result set from a query to the pg_rewrite table
	 * @param rewriteRule has the view definition to be rewritten: 1, else 0
	 */
	public void setRewriteRuleInfos(ResultSet rs, int rewriteRule) {
		try {
			ruleId = rs.getInt(2);
			event = Character.getNumericValue(rs.getString(4).charAt(0));
			attrno = rs.getInt(3);
			enabled = rs.getString(5).charAt(0);
			isInstead = rs.getBoolean(6);
			actions = rs.getString(8);
			this.rewriteRule = rewriteRule;
		} catch (SQLException e) {
			log.error("Error getting the infos for the rewrite rule");
			log.error(e.getMessage());
		}
	}

}
