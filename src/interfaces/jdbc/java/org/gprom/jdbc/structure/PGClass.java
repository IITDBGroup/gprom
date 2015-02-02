package org.gprom.jdbc.structure;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.log4j.Logger;
import org.gprom.jdbc.catalog.hsqldb.HSQLDBCatalogLookup;
import org.gprom.jdbc.container.DBCatalogTopContainer;

/**
 * Java implementation of the struct Form_pg_class
 * 
 * @author alex
 * 
 */
public class PGClass {
	private static Logger log = Logger.getLogger(PGClass.class);

	private String relName; /* class name */
	private int relNamespace; /* OID of namespace containing this class */
	private int relType; /* OID of associated entry in pg_type */
	private int relOwner; /* class owner */
	private int relam; /* index access method; 0 if not an index */
	private int relfilenode; /* identifier of physical storage file */
	private int reltablespace; /* identifier of table space for relation */
	private int relpages; /* # of blocks (not always up-to-date) */
	private float reltuples; /* # of tuples (not always up-to-date) */
	private int reltoastrelid; /* OID of toast table; 0 if none */
	private int reltoastidxid; /* if toast table, OID of chunk_id index */
	private boolean relhasindex; /* T if has (or has had) any indexes */
	private boolean relisshared; /* T if shared across databases */
	private char relkind; /* see RELKIND_xxx constants below */
	private int relnatts; /* number of user attributes */

	/*
	 * Class pg_attribute must contain exactly "relnatts" user attributes (with
	 * attnums ranging from 1 to relnatts) for this class.
	 */
	private int relchecks; /* # of CHECK constraints for class */
	private int relukeys; /* # of Unique keys (not used) */
	private int relfkeys; /* # of FOREIGN KEYs (not used) */
	private int relrefs; /* # of references to this rel (not used) */
	private boolean relhasoids; /* T if we generate OIDs for rows of rel */
	private boolean relhaspkey = false; /* has PRIMARY KEY index */
	private boolean relhasrules; /* has associated rules */
	private boolean relhassubclass; /* has derived classes */

	// Rewrite constants
	private String rewriteRelname = "PG_REWRITE";
	private int rewriteRelnamespace = 11;
	private int rewriteRelowner = 10;
	private int rewriteReltype = 10714;
	private int rewriteRelfilenode = 2618;
	private int rewriteRelpages = 9;
	private float rewriteTuples = 75;
	private int rewriteReltoastrelid = 0;
	private char rewriteRelkind = 'r';
	private int rewriteRelnatts = 8;

	private String rewriteIndexRelname = "PG_REWRITE_REL_RULENAME_INDEX";
	private int rewriteIndexRelnamespace = 11;
	private int rewriteIndexRelowner = 10;
	private int rewriteIndexReltype = 0;
	private int rewriteIndexRelfilenode = 2693;
	private int rewriteIndexRelpages = 2;
	private float rewriteIndexTuples = 75;
	private int rewriteIndexReltoastrelid = 0;
	private char rewriteIndexRelkind = 'i';
	private int rewriteIndexRelnatts = 2;

	/**
	 * Default Constructor
	 */
	public PGClass(){
		
	}
	
	/**
	 * Initializes all the fields of Form_pg_class for an OID
	 * 
	 * @param oid
	 *            the oid of a relation
	 * @param lookup
	 *            the catalog lookup object
	 * @param topContainer
	 *            the top container which holds all the OIDS
	 */
	public PGClass(int oid, HSQLDBCatalogLookup lookup,
			DBCatalogTopContainer topContainer) {
		if (oid != rewriteRelfilenode && oid != rewriteIndexRelfilenode) {
			relName = topContainer.getObjectByOID(oid).toUpperCase();
			ResultSet rs;
			try {
				rs = lookup.getColumnByRelname(relName);
				relnatts = 0;
				while (rs.next()) {
					relnatts++;
					relNamespace = topContainer.getOIDByName(rs.getString(2));
				}
				relType = rs.getType();

				relOwner = topContainer.putObjectInMap(lookup.getUserName().toUpperCase());
				// Check if a PK exits
				relhaspkey = lookup.checkForPrimaryKeys(relName);

				if (lookup.isView(relName)) {
					relkind = 'v';
					relhasrules = true;
				} else {
					relkind = 'r';
					relhasrules = false;
				}
			} catch (SQLException e) {
				log
						.error("Something went wrong while getting the meta data from"
								+ relName);
				log.error(e.getMessage());
				relNamespace = 0;
				relnatts = 0;
				relType = -1;
				relOwner = -1;
				relhaspkey = false;
				relkind = 'r';
				relhasrules = false;
			}
			relam = 0;
			relfilenode = 0;
			reltablespace = 0;
			relpages = 0;

			reltuples = 0;
			reltoastrelid = 0;
			reltoastidxid = 0;
			relhasindex = false;
			relisshared = false;

			relchecks = 0;
			relukeys = 0;
			relfkeys = 0;
			relrefs = 0;
			relhasoids = false;

			relhassubclass = false;
		} else if (oid == rewriteRelfilenode) {
			createRewriteRelation();
			topContainer.putObjectInMapByOid(rewriteRelfilenode, relName);
			relam = 0;
			reltablespace = 0;
			reltoastidxid = 0;
			relhasindex = false;
			relisshared = false;
			relchecks = 0;
			relukeys = 0;
			relfkeys = 0;
			relrefs = 0;
			relhasoids = false;
			relhaspkey = false;
			relhasrules = false;
			relhassubclass = false;
		} else if (oid == rewriteIndexRelfilenode) {
			createRewriteIndexRelation();
			topContainer.putObjectInMapByOid(rewriteRelfilenode, relName);
			relam = 0;
			reltablespace = 0;
			reltoastidxid = 0;
			relhasindex = false;
			relisshared = false;
			relchecks = 0;
			relukeys = 0;
			relfkeys = 0;
			relrefs = 0;
			relhasoids = false;
			relhaspkey = false;
			relhasrules = false;
			relhassubclass = false;
		}
		// topContainer.putStructObjectByOid(oid,this);
	}

	public int getRelationType(int typeId) {
		return 0;

	}

	private void createRewriteRelation() {
		relName = rewriteRelname;
		relOwner = rewriteRelowner;
		relNamespace = rewriteRelnamespace;
		relType = rewriteReltype;
		relfilenode = rewriteRelfilenode;
		relpages = rewriteRelpages;
		reltuples = rewriteTuples;
		reltoastrelid = rewriteReltoastrelid;
		relkind = rewriteRelkind;
		relnatts = rewriteRelnatts;
	}

	private void createRewriteIndexRelation() {
		relName = rewriteIndexRelname;
		relOwner = rewriteIndexRelowner;
		relNamespace = rewriteIndexRelnamespace;
		relType = rewriteIndexReltype;
		relfilenode = rewriteIndexRelfilenode;
		relpages = rewriteIndexRelpages;
		reltuples = rewriteIndexTuples;
		reltoastrelid = rewriteIndexReltoastrelid;
		relkind = rewriteIndexRelkind;
		relnatts = rewriteIndexRelnatts;
	}
	
	/**
	 * Sets all the attributes from a relation in the pg_class entry
	 * @param rs the ResultSet from a query to pg_class table
	 */
	public void setPGClassInfos(ResultSet rs){
		try {
			relName = rs.getString(1);
			relNamespace = rs.getInt(2);
			relType = rs.getInt(3);
			relOwner = rs.getInt(4);
			relam = rs.getInt(5);
			relfilenode = rs.getInt(6);
			reltablespace = rs.getInt(7);
			relpages = rs.getInt(8);
			reltuples = rs.getFloat(9);
			reltoastrelid = rs.getInt(10);
			reltoastidxid =rs.getInt(11);
			relhasindex = rs.getBoolean(12);
			relisshared =rs.getBoolean(13);
			
			relkind = rs.getString(14).charAt(0);
			relnatts = rs.getInt(15);
			relchecks = rs.getInt(16);
			relhasoids = rs.getBoolean(21);
			relhaspkey = rs.getBoolean(22);
			relhasrules =rs.getBoolean(23);
			relhassubclass = rs.getBoolean(24);

		} catch (SQLException e) {
			log.error("Error putting the pg_class information to the java attributes");
			log.error(e.getMessage());
			System.exit(-1);
		}
	}
	
	//GETTER METHODS
	public static Logger getLog() {
		return log;
	}
	public String getRelName() {
		return relName;
	}
	public int getRelNamespace() {
		return relNamespace;
	}
	public int getRelType() {
		return relType;
	}
	public int getRelOwner() {
		return relOwner;
	}
	public int getRelam() {
		return relam;
	}
	public int getRelfilenode() {
		return relfilenode;
	}
	public int getReltablespace() {
		return reltablespace;
	}
	public int getRelpages() {
		return relpages;
	}
	public float getReltuples() {
		return reltuples;
	}
	public int getReltoastrelid() {
		return reltoastrelid;
	}
	public int getReltoastidxid() {
		return reltoastidxid;
	}
	public boolean isRelhasindex() {
		return relhasindex;
	}
	public boolean isRelisshared() {
		return relisshared;
	}
	public char getRelkind() {
		return relkind;
	}
	public int getRelnatts() {
		return relnatts;
	}
	public int getRelchecks() {
		return relchecks;
	}
	public int getRelukeys() {
		return relukeys;
	}
	public int getRelfkeys() {
		return relfkeys;
	}
	public int getRelrefs() {
		return relrefs;
	}
	public boolean isRelhasoids() {
		return relhasoids;
	}
	public boolean isRelhaspkey() {
		return relhaspkey;
	}
	public boolean isRelhasrules() {
		return relhasrules;
	}
	public boolean isRelhassubclass() {
		return relhassubclass;
	}
}
