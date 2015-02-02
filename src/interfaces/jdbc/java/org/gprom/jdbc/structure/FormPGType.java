package org.gprom.jdbc.structure;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.log4j.Logger;
import org.gprom.jdbc.catalog.hsqldb.HSQLDBCatalogLookup;
import org.gprom.jdbc.catalog.hsqldb.HSQLDBInterface;
import org.gprom.jdbc.container.DBCatalogTopContainer;

/**
 * Java Implementation of the Form_pg_type
 * @author alex
 *
 */
public class FormPGType {
	
	private static Logger log = Logger.getLogger(HSQLDBInterface.class);
	
	
	
	String	 	typname;		/* type name */

	/*
	 * If typelem is not 0 then it identifies another row in pg_type. The
	 * current type can then be subscripted like an array yielding values of
	 * type typelem. A non-zero typelem does not guarantee this type to be a
	 * "real" array type; some ordinary fixed-length types can also be
	 * subscripted (e.g., name, point). Variable-length types can *not* be
	 * turned into pseudo-arrays like that. Hence, the way to determine
	 * whether a type is a "true" array type is if:
	 *
	 * typelem != 0 and typlen == -1.
	 */
	int			typelem;
	
	/* ----------------
	 * typstorage tells if the type is prepared for toasting and what
	 * the default strategy for attributes of this type should be.
	 *
	 * 'p' PLAIN	  type not prepared for toasting
	 * 'e' EXTERNAL   external storage possible, don't try to compress
	 * 'x' EXTENDED   try to compress and store external if required
	 * 'm' MAIN		  like 'x' but try to keep in main tuple
	 * ----------------
	 */
	char		typstorage;
	
	/*
	 * typtype is 'b' for a base type, 'c' for a composite type (e.g., a
	 * table's rowtype), 'd' for a domain type, 'e' for an enum type, or 'p'
	 * for a pseudo-type.  (Use the TYPTYPE macros below.)
	 *
	 * If typtype is 'c', typrelid is the OID of the class' entry in pg_class.
	 */
	char		typtype;
	
	/*
	 * I/O conversion procedures for the datatype.
	 */
	int		typoutput;
	
	
	
/*######################################################################################## */
//TEMPORARLY UNUSED VARIABLE
/*	
  	int			typnamespace;	* OID of namespace containing this type 
  	int			typowner;		* type owner 
  	
	 * For a fixed-size type, typlen is the number of bytes we use to
	 * represent a value of this type, e.g. 4 for an int4.	But for a
	 * variable-length type, typlen is negative.  We use -1 to indicate a
	 * "varlena" type (one that has a length word), -2 to indicate a
	 * null-terminated C string.
	 
	int		typlen;

	
	 * typbyval determines whether internal Postgres routines pass a value of
	 * this type by value or by reference.	typbyval had better be FALSE if
	 * the length is not 1, 2, or 4 (or 8 on 8-byte-Datum machines).
	 * Variable-length types are always passed by reference. Note that
	 * typbyval can be false even if the length would allow pass-by-value;
	 * this is currently true for type float4, for example.
	 
	bool		typbyval;



	
	 * If typisdefined is false, the entry is only a placeholder (forward
	 * reference).	We know the type name, but not yet anything else about it.
	 
	bool		typisdefined;

	char		typdelim;		 delimiter for arrays of this type 

	Oid			typrelid;		 0 if not a complex type 



	
	 * If there is a "true" array type having this type as element type,
	 * typarray links to it.  Zero if no associated "true" array type.
	 
	Oid			typarray;

	
	 * I/O conversion procedures for the datatype.
	 
	regproc		typinput;		 text format (required) 
	
	regproc		typreceive;		 binary format (optional) 
	regproc		typsend;

	
	 * I/O functions for optional type modifiers.
	 
	regproc		typmodin;
	regproc		typmodout;

	
	 * Custom ANALYZE procedure for the datatype (0 selects the default).
	 
	regproc		typanalyze;

	 ----------------
	 * typalign is the alignment required when storing a value of this
	 * type.  It applies to storage on disk as well as most
	 * representations of the value inside Postgres.  When multiple values
	 * are stored consecutively, such as in the representation of a
	 * complete row on disk, padding is inserted before a datum of this
	 * type so that it begins on the specified boundary.  The alignment
	 * reference is the beginning of the first datum in the sequence.
	 *
	 * 'c' = CHAR alignment, ie no alignment needed.
	 * 's' = SHORT alignment (2 bytes on most machines).
	 * 'i' = INT alignment (4 bytes on most machines).
	 * 'd' = DOUBLE alignment (8 bytes on many machines, but by no means all).
	 *
	 * See include/access/tupmacs.h for the macros that compute these
	 * alignment requirements.	Note also that we allow the nominal alignment
	 * to be violated when storing "packed" varlenas; the TOAST mechanism
	 * takes care of hiding that from most code.
	 *
	 * NOTE: for types used in system tables, it is critical that the
	 * size and alignment defined in pg_type agree with the way that the
	 * compiler will lay out the field in a struct representing a table row.
	 * ----------------
	 
	char		typalign;



	
	 * This flag represents a "NOT NULL" constraint against this datatype.
	 *
	 * If true, the attnotnull column for a corresponding table column using
	 * this datatype will always enforce the NOT NULL constraint.
	 *
	 * Used primarily for domain types.
	 
	bool		typnotnull;

	
	 * Domains use typbasetype to show the base (or domain) type that the
	 * domain is based on.	Zero if the type is not a domain.
	 
	Oid			typbasetype;

	
	 * Domains use typtypmod to record the typmod to be applied to their base
	 * type (-1 if base type does not use a typmod).  -1 if this type is not a
	 * domain.
	 
	int4		typtypmod;

	
	 * typndims is the declared number of dimensions for an array domain type
	 * (i.e., typbasetype is an array type; the domain's typelem will match
	 * the base type's typelem).  Otherwise zero.
	 
	int4		typndims;

	
	 * If typdefaultbin is not NULL, it is the nodeToString representation of
	 * a default expression for the type.  Currently this is only used for
	 * domains.
	 
	text		typdefaultbin;	 VARIABLE LENGTH FIELD 

	
	 * typdefault is NULL if the type has no associated default value. If
	 * typdefaultbin is not NULL, typdefault must contain a human-readable
	 * version of the default expression represented by typdefaultbin. If
	 * typdefaultbin is NULL and typdefault is not, then typdefault is the
	 * external representation of the type's default value, which may be fed
	 * to the type's input converter to produce a constant.
	 
	text		typdefault;		 VARIABLE LENGTH FIELD 
	*/
	/*######################################################################################## */
	
	public FormPGType(int typeid,HSQLDBCatalogLookup lookUp, DBCatalogTopContainer topContainer){
		if(topContainer.getTypeHolder().getTypeNameForID(typeid) != null){
			typname = topContainer.getTypeHolder().getTypeNameForID(typeid);
			
		}else{
			typname = "VARCHAR";
		}
		typelem = 0;
		typstorage = 'p';
		
		ResultSet rs;
		try {
			rs = lookUp.getTypeInfo();
			while(rs.next()){
				if(rs.getString("TYPE_NAME").equals(typname.toUpperCase())){
					typtype = 'b';
				}
			}
		} catch (SQLException e) {
			log.error(e.getMessage());
		}
		typoutput = topContainer.getTypeHolder().getTypeID(typname);
	}
}
