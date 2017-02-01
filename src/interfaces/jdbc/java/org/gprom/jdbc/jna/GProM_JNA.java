/**
 * 
 */
package org.gprom.jdbc.jna;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.NativeLong;
import com.sun.jna.Pointer;
import com.sun.jna.StringArray;
import com.sun.jna.Structure;
import com.sun.jna.Union;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

/**
 * @author lord_pretzel
 *
 */
public interface GProM_JNA extends Library {
	
	// instance of the library
	GProM_JNA INSTANCE = (GProM_JNA) Native.loadLibrary("gprom", 
			GProM_JNA.class);

	// library methods
	// initialization
	public void gprom_init();
	public void gprom_readOptions(int argc, String[] args);
	public void gprom_readOptionAndInit(int argc, String[] args);
	public void gprom_configFromOptions();
	public void gprom_reconfPlugins();
	public void gprom_shutdown();
	
	// rewrite methods
    public Pointer gprom_rewriteQuery (String query);
    
    // configuration interface
    public String gprom_getStringOption (String name);
    public int gprom_getIntOption (String name);
    public boolean gprom_getBoolOption (String name);
    public double gprom_getFloatOption (String name);

    public void gprom_setStringOption (String name, String value);
    public void gprom_setIntOption(String name, int value);
    public void gprom_setBoolOption(String name, boolean value);
    public void gprom_setFloatOption(String name, double value);	

    public String gprom_getOptionType(String key);
    public boolean gprom_optionExists(String key);
    
    // logging  callback interface
    interface GProMLoggerCallbackFunction extends Callback {
        void invoke(String message, String fileInfo, int line, int logLevel);
    }
    
    // exception callback interface
    interface GProMExceptionCallbackFunction extends Callback {
    	int invoke(String message, String file, int line, int severity);
    }
    
    public void gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback);
	public void gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback);
	public void gprom_setMaxLogLevel (int maxLevel);
	
	// metadata lookup callback interface defines a methods and a plugin struct
//	public class GProMMetadataLookupPlugin extends Structure {
//		
//		/// C type : boolean isInitialized
//		public GProMMetadataLookupPlugin.isInitialized_callback isInitialized;
//		/// C type : initMetadataLookupPlugin_callback
//		public GProMMetadataLookupPlugin.initMetadataLookupPlugin_callback initMetadataLookupPlugin;
//		/// C type : databaseConnectionOpen_callback
//		public GProMMetadataLookupPlugin.databaseConnectionOpen_callback databaseConnectionOpen;
//		/// C type : databaseConnectionClose_callback
//		public GProMMetadataLookupPlugin.databaseConnectionClose_callback databaseConnectionClose;
//		/// C type : shutdownMetadataLookupPlugin_callback
//		public GProMMetadataLookupPlugin.shutdownMetadataLookupPlugin_callback shutdownMetadataLookupPlugin;
//	    /// C type: boolean (*catalogTableExists) (char * tableName);
//		public GProMMetadataLookupPlugin.catalogTableExists_callback catalogTableExists;
//		/// C type: boolean (*catalogViewExists) (char * viewName);
//		public GProMMetadataLookupPlugin.catalogViewExists_callback catalogViewExists;
//		/// C type : getAttributes_callback		
//		public GProMMetadataLookupPlugin.getAttributes_callback getAttributes;
//		/// C type : getAttributeNames_callback
//		public GProMMetadataLookupPlugin.getAttributeNames_callback getAttributeNames;
//		/// C type : getAttributeDefaultVal_callback
//		public GProMMetadataLookupPlugin.getAttributeDefaultVal_callback getAttributeDefaultVal;
//		/// C type : boolean (*isAgg) (char *functionName)
//		public GProMMetadataLookupPlugin.isAgg_callback isAgg;
//	    /// C types : boolean (*isWindowFunction) (char *functionName)
//		public GProMMetadataLookupPlugin.isWindowFunction_callback isWindowFunction;
//		/// C type : getFuncReturnType_callback
//		public GProMMetadataLookupPlugin.getFuncReturnType_callback getFuncReturnType;
//		/// C type : getOpReturnType_callback
//		public GProMMetadataLookupPlugin.getOpReturnType_callback getOpReturnType;
//		/// C type : getTableDefinition_callback
//		public GProMMetadataLookupPlugin.getTableDefinition_callback getTableDefinition;
//		/// C type : getViewDefinition_callback
//		public GProMMetadataLookupPlugin.getViewDefinition_callback getViewDefinition;
//		/// C type:   char ** (*getKeyInformation) (char *tableName)
//		public GProMMetadataLookupPlugin.getKeyInformation_callback getKeyInformation;
//
//		
//		public interface isInitialized_callback {
//			boolean apply();
//		}
//		public interface initMetadataLookupPlugin_callback extends Callback {
//			int apply();
//		};
//		public interface databaseConnectionOpen_callback extends Callback {
//			int apply();
//		};
//		public interface databaseConnectionClose_callback extends Callback {
//			int apply();
//		};
//		public interface shutdownMetadataLookupPlugin_callback extends Callback {
//			int apply();
//		};
//		public interface catalogViewExists_callback extends Callback {
//			int apply(String viewName);
//		}
//		public interface catalogTableExists_callback extends Callback {
//			int apply(String tableName);
//		}
//		public interface getAttributes_callback extends Callback {
//			void apply(String tableName, 
//					PointerByReference attrs, 	// output parameter char *** 
//					PointerByReference dataTypes, // output parameter char ***
//					IntByReference numArgs);	// output parameter int *	
//		};
//		public interface getAttributeNames_callback extends Callback {
//			void apply(String tableName, 
//					PointerByReference attrs, 	// output parameter char *** 
//					IntByReference numArgs 		// output parameter int *
//					);
//		};
//		public interface getAttributeDefaultVal_callback extends Callback {
//			String apply(String schema, String  tableName, String attrName);
//		};
//		public interface isWindowFunction_callback {
//			int apply(String functionName);
//		}
//		public interface isAgg_callback {
//			int apply(String functionName);
//		}
//		public interface getFuncReturnType_callback extends Callback {
//			String apply(String fName, StringArray args, int numArgs);
//		};
//		public interface getOpReturnType_callback extends Callback {
//			String apply(String oName, StringArray args, int numArgs);
//		};
//		public interface getTableDefinition_callback extends Callback {
//			String apply(String tableName);
//		};
//		public interface getViewDefinition_callback extends Callback {
//			String apply(String viewName);
//		};
//		public interface getKeyInformation_callback extends Callback {
//			PointerByReference apply(String tableName);
//		};
//		
//		public GProMMetadataLookupPlugin() {
//			super();
//			initFieldOrder();
//		}
//		protected void initFieldOrder() {
//			setFieldOrder(new String[]{"isInitialized", "initMetadataLookupPlugin", "databaseConnectionOpen", "databaseConnectionClose", "shutdownMetadataLookupPlugin", "getAttributes", "getAttributeNames", "getAttributeDefaultVal", "getFuncReturnType", "getOpReturnType", "getTableDefinition", "getViewDefinition"});
//		}
//		protected ByReference newByReference() { return new ByReference(); }
//		protected ByValue newByValue() { return new ByValue(); }
//		protected GProMMetadataLookupPlugin newInstance() { return new GProMMetadataLookupPlugin(); }
////		public static GProMMetadataLookupPlugin[] newArray(int arrayLength) {
////			return Structure.newArray(GProMMetadataLookupPlugin.class, arrayLength);
////		}
//		public static class ByReference extends GProMMetadataLookupPlugin implements Structure.ByReference {
//			
//		};
//		public static class ByValue extends GProMMetadataLookupPlugin implements Structure.ByValue {
//			
//		}
//		/* (non-Javadoc)
//		 * @see com.sun.jna.Structure#getFieldOrder()
//		 */
//		@Override
//		protected List getFieldOrder() {
//			// TODO Auto-generated method stub
//			return null;
//		};
//	}

	void gprom_registerMetadataLookupPlugin(GProMMetadataLookupPlugin plugin);

	
	public static interface GProMNodeTag {
		public static final int GProM_T_Invalid = 0;
		public static final int GProM_T_Node = 1;

	    /* collection types */
	    public static final int GProM_T_List = 2;
	    public static final int GProM_T_IntList = 3;
	    public static final int GProM_T_Set = 4;
	    public static final int GProM_T_HashMap = 5;
	    public static final int GProM_T_Vector = 6;

	    /* options */
	    public static final int GProM_T_KeyValue = 7;

	    /* expression nodes */
		public static final int GProM_T_Constant = 8;
	    public static final int GProM_T_AttributeReference = 9;
	    public static final int GProM_T_SQLParameter = 10;
	    public static final int GProM_T_FunctionCall = 11;
	    public static final int GProM_T_Operator = 12;
	    public static final int GProM_T_CaseExpr = 13;
	    public static final int GProM_T_CaseWhen = 14;
	    public static final int GProM_T_IsNullExpr = 15;
	    public static final int GProM_T_WindowBound = 16;
	    public static final int GProM_T_WindowFrame = 17;
	    public static final int GProM_T_WindowDef = 18;
	    public static final int GProM_T_WindowFunction = 19;
	    public static final int GProM_T_RowNumExpr = 20;
	    public static final int GProM_T_OrderExpr = 21;
	    public static final int GProM_T_CastExpr = 22;

	    /* query block model nodes */
	    public static final int GProM_T_SetQuery = 23;
	    public static final int GProM_T_ProvenanceStmt = 24;
	    public static final int GProM_T_ProvenanceTransactionInfo = 25;
	    public static final int GProM_T_QueryBlock = 26;
	    public static final int GProM_T_SelectItem = 27;
	    public static final int GProM_T_FromItem = 28;
	    public static final int GProM_T_FromProvInfo = 29;
	    public static final int GProM_T_FromTableRef = 30;
	    public static final int GProM_T_FromSubquery = 31;
	    public static final int GProM_T_FromJoinExpr = 32;
	    public static final int GProM_T_DistinctClause = 33;
	    public static final int GProM_T_NestedSubquery = 34;
	    public static final int GProM_T_Insert = 35;
	    public static final int GProM_T_Delete = 36;
	    public static final int GProM_T_Update = 37;
	    public static final int GProM_T_TransactionStmt = 38;
	    public static final int GProM_T_WithStmt = 39;
	    public static final int GProM_T_DDLStatement = 40;
	    public static final int GProM_T_UtilityStatement = 41;

	    /* query operator model nodes */
	    public static final int GProM_T_Schema = 42;
	    public static final int GProM_T_AttributeDef = 43;
	    public static final int GProM_T_QueryOperator = 44;
	    public static final int GProM_T_SelectionOperator = 45;
	    public static final int GProM_T_ProjectionOperator = 46;
	    public static final int GProM_T_JoinOperator = 47;
	    public static final int GProM_T_AggregationOperator = 48;
	    public static final int GProM_T_ProvenanceComputation = 49;
	    public static final int GProM_T_TableAccessOperator = 50;
	    public static final int GProM_T_SetOperator = 51;
	    public static final int GProM_T_DuplicateRemoval = 52;
	    public static final int GProM_T_ConstRelOperator = 53;
	    public static final int GProM_T_NestingOperator = 54;
	    public static final int GProM_T_WindowOperator = 55;
	    public static final int GProM_T_OrderOperator = 56;

	    /* datalog model nodes */
	    public static final int GProM_T_DLNode = 57;
	    public static final int GProM_T_DLAtom = 58;
	    public static final int GProM_T_DLVar = 59;
	    public static final int GProM_T_DLRule = 60;
	    public static final int GProM_T_DLProgram = 61;
	    public static final int GProM_T_DLComparison = 62;
		public static final int GProM_T_DLDomain = 63;

	    /* Json Table Node */
	    public static final int GProM_T_FromJsonTable = 64;
	    public static final int GProM_T_JsonTableOperator = 65;
	    public static final int GProM_T_JsonColInfoItem = 66;
	    public static final int GProM_T_JsonPath = 67;

	    /* relation */
	    public static final int GProM_T_Relation = 68;

	    /* rpq */
	    public static final int GProM_T_Regex = 69;
	    public static final int GProM_T_RPQQuery = 70;

	    /* ddl */
	    public static final int GProM_T_CreateTable = 71;
	    public static final int GProM_T_AlterTable = 72;
	}
	
	public static interface GProMDataType {
		public static final int GProM_DT_INT = 0;
		public static final int GProM_DT_LONG = 1;
		public static final int GProM_DT_STRING = 2;
		public static final int GProM_DT_FLOAT = 3;
		public static final int GProM_DT_BOOL = 4;
		public static final int GProM_DT_VARCHAR2 = 5;
	}
	
	public static interface GProMJoinType {
	    public static final int GProM_JOIN_INNER = 0;
	    public static final int GProM_JOIN_CROSS = 1;
	    public static final int GProM_JOIN_LEFT_OUTER = 2;
	    public static final int GProM_JOIN_RIGHT_OUTER = 3;
	    public static final int GProM_JOIN_FULL_OUTER = 4;
	} 
	
	public static interface GProMSetOpType{
	        public static final int GProM_SETOP_UNION = 0;
	        public static final int GProM_SETOP_INTERSECTION = 1;
	        public static final int GProM_SETOP_DIFFERENCE = 2;
	} 
	
	public static interface GProMProvenanceType {
	    public static final int GProM_PROV_PI_CS = 0;
	    public static final int GProM_PROV_TRANSFORMATION = 1;
	    public static final int GProM_PROV_NONE = 2;/* for reenactment of bag semantics only */
	} 
	
	/* what type of database operation(s) a provenance computation is for */
	public static interface GProMProvenanceInputType {
	    public static final int GProM_PROV_INPUT_QUERY = 0;
	    public static final int GProM_PROV_INPUT_UPDATE = 1;
	    public static final int GProM_PROV_INPUT_UPDATE_SEQUENCE = 2;
	    public static final int GProM_PROV_INPUT_REENACT = 3;
	    public static final int GProM_PROV_INPUT_REENACT_WITH_TIMES = 4;
	    public static final int GProM_PROV_INPUT_TRANSACTION = 5;
	    public static final int GProM_PROV_INPUT_TIME_INTERVAL = 6;
	} 
	
	public static interface GProMIsolationLevel {
	    public static final int GProM_ISOLATION_SERIALIZABLE = 0;
	    public static final int GProM_ISOLATION_READ_COMMITTED = 1;
	    public static final int GProM_ISOLATION_READ_ONLY = 2;
	} 
	
	public static interface GProMNestingExprType {
		    public static final int GProM_NESTQ_EXISTS = 0;
		    public static final int GProM_NESTQ_ANY = 1;
		    public static final int GProM_NESTQ_ALL = 2;
		    public static final int GProM_NESTQ_UNIQUE = 3;
		    public static final int GProM_NESTQ_SCALAR = 4;
	} 
	
	public static interface GProMWindowBoundType {
		public static final int GProM_WINBOUND_UNBOUND_PREC = 0;
		public static final int GProM_WINBOUND_CURRENT_ROW = 1;
		public static final int GProM_WINBOUND_EXPR_PREC = 2;
		public static final int GProM_WINBOUND_EXPR_FOLLOW = 3;
	}
	
	public static interface GProMWinFrameType {
		public static final int GProM_WINFRAME_ROWS = 0;
		public static final int GProM_WINFRAME_RANGE = 1;
	}
	
	public static interface GProMSortOrder {
		public static final int GProM_SORT_ASC = 0;
		public static final int GProM_SORT_DESC = 1;
	}
	
	public static interface GProMSortNullOrder {
		public static final int GProM_SORT_NULLS_FIRST = 0;
		public static final int GProM_SORT_NULLS_LAST = 1;
	}
	
	public static final int GProM_INVALID_PARAM = (int)-1;
	public static final int GProM_INVALID_ATTR = (int)-1;
	public static final int GProM_INVALID_FROM_ITEM = (int)-1;
	
	public Pointer gprom_rewriteQueryToOperatorModel (String query);
    
	public Pointer gprom_provRewriteOperator(Pointer nodeFromMimir);

}
