package org.gprom.jdbc.structure;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.log4j.Logger;

import org.gprom.jdbc.container.DBCatalogTopContainer;
import org.gprom.jdbc.container.TypeHolder;


/**
 * Java implementation of the Form_pg_operator
 * 
 * @author alex
 * 
 */
public class FormPGOperator {

	private static Logger log = Logger.getLogger(RewriteRuleClass.class);

	private String oprname; /* name of operator */
	private int oprnamespace; /* OID of namespace containing this oper */
	private int oprowner; /* operator owner */
	private char oprkind; /* 'l', 'r', or 'b' */
	private boolean oprcanmerge; /* can be used in merge join? */
	private boolean oprcanhash; /* can be used in hash join? */
	private int oprleft; /* left arg type, or 0 if 'l' oprkind */
	private int oprright; /* right arg type, or 0 if 'r' oprkind */
	private int oprresult; /* result datatype */
	private int oprcom; /* OID of commutator oper, or 0 if none */
	private int oprnegate; /* OID of negator oper, or 0 if none */
	private int oprcode; /* OID of underlying function */
	private int oprrest; /* OID of restriction estimator, or 0 */
	private int oprjoin; /* OID of join estimator, or 0 */

	/**
	 * Creates an operator for the struct form_pg_operator
	 * 
	 * @param oid
	 *            the oid of the operator
	 * @param ltypeId
	 *            the left type id of the operator
	 * @param rtypeId
	 *            the right type id of the operator
	 * @param operkind
	 *            the operator kind 'l','r', 'b'
	 * @param topContainer
	 *            the id container
	 */
	public FormPGOperator(int oid, int ltypeId, int rtypeId, char operkind,
			DBCatalogTopContainer topContainer) {
		oprname = topContainer.getObjectByOID(oid).trim();
		oprnamespace = oid;
		oprkind = operkind;
		if (operkind == 'b') {
			oprresult = topContainer.getOIDByName("BOOLEAN");
			oprleft = ltypeId;
			oprright = rtypeId;
		} else {
			oprresult = topContainer.getOIDByName("STRING");
			if (operkind == 'l') {
				oprleft = 0;
				oprright = rtypeId;
			} else {
				oprleft = ltypeId;
				oprright = 0;
			}
		}
		oprowner = 10;
		oprcanmerge = true;
		oprcanhash = true;
		oprcom = 0;
		oprnegate = 0;
		oprcode = 0;
		oprrest = 0;
		oprjoin = 0;
	}

	/**
	 * Creates an equality operator
	 * 
	 * @param argType
	 *            the argType of left and right type
	 * @param typeHolder
	 *            the type holder map
	 */
	public FormPGOperator(int leftType,int rightType, TypeHolder typeHolder) {
		oprname = "=";
		oprnamespace = 44;
		oprkind = 'b';
		// Put the text ids for left and right operator or the nummeric id
		if (typeHolder.isText(leftType)) {
			oprleft = 25;
			oprright = 25;
		} else {
			oprleft = 1700;
			oprright = 1700;
		}
		oprowner = 10;
		oprcanmerge = true;
		oprcanhash = true;
		oprcom = 0;
		oprnegate = 0;
		oprcode = 0;
		oprrest = 0;
		oprjoin = 0;
	}

	public FormPGOperator() {
	}

	public void setPGOperatorInfos(ResultSet rs) {
		try {
			oprname = rs.getString("oprname");
			oprnamespace = rs.getInt("oprnamespace");
			oprowner = rs.getInt("oprowner");
			oprkind = rs.getString("oprkind").charAt(0);
			oprcanmerge = rs.getBoolean("oprcanmerge");
			oprcanhash = rs.getBoolean("oprcanhash");
			oprleft = rs.getInt("oprleft");
			oprright = rs.getInt("oprright");
			oprresult = rs.getInt("oprresult");
			oprcom = rs.getInt("oprcom");
			oprnegate = rs.getInt("oprnegate");
			oprcode = 0;
			oprrest = 0;
			oprjoin = 0;
		} catch (SQLException e) {
			log.error("Error getting the operator information");
			log.error(e.getMessage());
		}
		
	}
	
	// GETTER METHODS
	public String getOprname() {
		return oprname;
	}

	public int getOprnamespace() {
		return oprnamespace;
	}

	public int getOprowner() {
		return oprowner;
	}

	public char getOprkind() {
		return oprkind;
	}

	public boolean isOprcanmerge() {
		return oprcanmerge;
	}

	public boolean isOprcanhash() {
		return oprcanhash;
	}

	public int getOprleft() {
		return oprleft;
	}

	public int getOprright() {
		return oprright;
	}

	public int getOprresult() {
		return oprresult;
	}

	public int getOprcom() {
		return oprcom;
	}

	public int getOprnegate() {
		return oprnegate;
	}

	public int getOprcode() {
		return oprcode;
	}

	public int getOprrest() {
		return oprrest;
	}

	public int getOprjoin() {
		return oprjoin;
	}


}
