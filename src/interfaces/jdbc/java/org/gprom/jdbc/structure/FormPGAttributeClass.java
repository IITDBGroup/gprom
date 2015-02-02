package org.gprom.jdbc.structure;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.log4j.Logger;
import org.gprom.jdbc.catalog.hsqldb.HSQLDBCatalogLookup;
import org.gprom.jdbc.container.DBCatalogTopContainer;

/**
 * Java implementation of the Form_pg_attribute struct
 * @author alex
 *
 */
public class FormPGAttributeClass{
	private static Logger log = Logger.getLogger(FormPGAttributeClass.class);
	
	/* OID of relation containing this attribute */
	private int attrelid;
	/* name of attribute */
	private String attname;
	/*
	 * atttypid defines the data type of this attribute (e.g. int4).  Information in
	 * that instance is redundant with the attlen, attbyval, and attalign
	 * attributes of this instance, so they had better match or Postgres will
	 * fail.
	 */
	private int atttypid;
	/*
	 * attstattarget is the target number of statistics datapoints to collect
	 * during VACUUM ANALYZE of this column.  A zero here indicates that we do
	 * not wish to collect any stats about this column. A "-1" here indicates
	 * that no value has been explicitly set for this column, so ANALYZE
	 * should use the default setting.
	 */
	private int attstatttarget;
	
	/*
	 * attlen is a copy of the typlen field from pg_type for this attribute.
	 * See atttypid comments above.
	 */
	private int attlen;
	/*
	 * attnum is the "attribute number" for the attribute:	A value that
	 * uniquely identifies this attribute within its class. For user
	 * attributes, Attribute numbers are greater than 0 and not greater than
	 * the number of attributes in the class. I.e. if the Class pg_class says
	 * that Class XYZ has 10 attributes, then the user attribute numbers in
	 * Class pg_attribute must be 1-10.
	 *
	 * System attributes have attribute numbers less than 0 that are unique
	 * within the class, but not constrained to any particular range.
	 *
	 * Note that (attnum - 1) is often used as the index to an array.
	 */
	private int attnum;

	/*
	 * attndims is the declared number of dimensions, if an array type,
	 * otherwise zero.
	 */
	private int attndims;
	/*
	 * fastgetattr() uses attcacheoff to cache byte offsets of attributes in
	 * heap tuples.  The value actually stored in pg_attribute (-1) indicates
	 * no cached value.  But when we copy these tuples into a tuple
	 * descriptor, we may then update attcacheoff in the copies. This speeds
	 * up the attribute walking process.
	 */
	private int attcacheoff;
	/*
	 * atttypmod records type-specific data supplied at table creation time
	 * (for example, the max length of a varchar field).  It is passed to
	 * type-specific input and output functions as the third argument. The
	 * value will generally be -1 for types that do not need typmod.
	 */
	private int atttypmod;
	/*
	 * attbyval is a copy of the typbyval field from pg_type for this
	 * attribute.  See atttypid comments above.
	 */
	private boolean attbyval;
	/*----------
	 * attstorage tells for VARLENA attributes, what the heap access
	 * methods can do to it if a given tuple doesn't fit into a page.
	 * Possible values are
	 *		'p': Value must be stored plain always
	 *		'e': Value can be stored in "secondary" relation (if relation
	 *			 has one, see pg_class.reltoastrelid)
	 *		'm': Value can be stored compressed inline
	 *		'x': Value can be stored compressed inline or in "secondary"
	 * Note that 'm' fields can also be moved out to secondary storage,
	 * but only as a last resort ('e' and 'x' fields are moved first).
	 *----------
	 */
	private char attstorage;
	/*
	 * attalign is a copy of the typalign field from pg_type for this
	 * attribute.  See atttypid comments above.
	 */
	private char attalign;
	/* This flag represents the "NOT NULL" constraint */
	private boolean attnotnull;
	/* Has DEFAULT value or not */
	private boolean atthasdef;
	/* Is dropped (ie, logically invisible) or not */
	private boolean attisdropped;
	/* Has a local definition (hence, do not drop when attinhcount is 0) */
	private boolean attislocal;
	/* Number of times inherited from direct parent relation(s) */
	private int attinhcount;
	
	/**
	 * Fills all the needed value with information for an attribute
	 * @param relationOID The relation id the attribute belongs to
	 * @param lookUp the catalog look up object
	 * @param topContainer the top container for all OIDS
	 * @param  index the attribute which has to be generated
	 */
	public FormPGAttributeClass(int relationOID, HSQLDBCatalogLookup lookUp, DBCatalogTopContainer topContainer, int index){
		attrelid = relationOID;
		String relName = topContainer.getObjectByOID(relationOID);
		try {
			ResultSet rs = lookUp.getColumnByRelname(relName);
			rs.relative(index);
			attname = rs.getString(4);
			atttypid = topContainer.getTypeHolder().getTypeID(rs.getString(6));
			attlen = topContainer.getTypeHolder().getTypeLength(atttypid);
			attnum = index;
			atttypmod = rs.getInt(7);
			if (rs.getInt(11) == 0){
				attnotnull = true;
			}else{
				attnotnull = false;
			}
			if(rs.getString(13) == null){
				atthasdef = false;
			}else{
				atthasdef = true;
			}
		} catch (SQLException e) {
			log.error("Something went wrong while getting the column meta data");
			log.error(e.getMessage());
			attname = null;
			atttypid = -1;
			attlen = -1;
			attnum = -1;
			atttypmod = -1;
			attnotnull = false;
			atthasdef = false;
		}
		
		attstatttarget = -1;
		attndims = 0;
		attcacheoff = -1;
		
		attbyval = false;
		attstorage = 'p';
		//No alignment
		attalign = 'c';
		
		attisdropped = false;
		attislocal = true;
		
		attinhcount = 0;
	}

	public FormPGAttributeClass() {
	}

	public void setPGAttributeInfos(ResultSet rs) {
		try {
			attrelid = rs.getInt(1);
			attname = rs.getString(2);
			atttypid = rs.getInt(3);
			attstatttarget = rs.getInt(4);
			attlen = rs.getInt(5);
			attnum = rs.getInt(6);
			attndims = rs.getInt(7);
			attcacheoff = rs.getInt(8);
			atttypmod = rs.getInt(9);
			attbyval = rs.getBoolean(10);
			attstorage = rs.getString(11).charAt(0);
			attalign = rs.getString(12).charAt(0);
			attnotnull = rs.getBoolean(13);
			atthasdef =rs.getBoolean(14);
			attisdropped = rs.getBoolean(15);
			attislocal = rs.getBoolean(16);
			attinhcount = rs.getInt(17);
		} catch (SQLException e) {
			log.error("Error setting the infos for the FormPGAttribute");
			log.error(e.getMessage());
		}
		
	}
	
//GETTER METHODS
	public int getAttrelid() {
		return attrelid;
	}

	public String getAttname() {
		return attname;
	}

	public int getAtttypid() {
		return atttypid;
	}

	public int getAttstatttarget() {
		return attstatttarget;
	}

	public int getAttlen() {
		return attlen;
	}

	public int getAttnum() {
		return attnum;
	}

	public int getAttndims() {
		return attndims;
	}

	public int getAttcacheoff() {
		return attcacheoff;
	}

	public int getAtttypmod() {
		return atttypmod;
	}

	public boolean isAttbyval() {
		return attbyval;
	}

	public char getAttstorage() {
		return attstorage;
	}

	public char getAttalign() {
		return attalign;
	}

	public boolean isAttnotnull() {
		return attnotnull;
	}

	public boolean isAtthasdef() {
		return atthasdef;
	}

	public boolean isAttisdropped() {
		return attisdropped;
	}

	public boolean isAttislocal() {
		return attislocal;
	}

	public int getAttinhcount() {
		return attinhcount;
	}
	
}
