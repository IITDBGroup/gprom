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

	public static final Object GC_LOCK = new Object();
	
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
		/** <i>native declaration : line 292</i> */
		public static final int GProM_T_Invalid = 0;
		/** <i>native declaration : line 293</i> */
		public static final int GProM_T_Node = 1;
		/**
		 * collection types<br>
		 * <i>native declaration : line 296</i>
		 */
		public static final int GProM_T_List = 2;
		/** <i>native declaration : line 297</i> */
		public static final int GProM_T_IntList = 3;
		/** <i>native declaration : line 298</i> */
		public static final int GProM_T_Set = 4;
		/** <i>native declaration : line 299</i> */
		public static final int GProM_T_HashMap = 5;
		/** <i>native declaration : line 300</i> */
		public static final int GProM_T_Vector = 6;
		/**
		 * options<br>
		 * <i>native declaration : line 303</i>
		 */
		public static final int GProM_T_KeyValue = 7;
		/**
		 * expression nodes<br>
		 * <i>native declaration : line 306</i>
		 */
		public static final int GProM_T_Constant = 8;
		/** <i>native declaration : line 307</i> */
		public static final int GProM_T_AttributeReference = 9;
		/** <i>native declaration : line 308</i> */
		public static final int GProM_T_SQLParameter = 10;
		/** <i>native declaration : line 309</i> */
		public static final int GProM_T_FunctionCall = 11;
		/** <i>native declaration : line 310</i> */
		public static final int GProM_T_Operator = 12;
		/** <i>native declaration : line 311</i> */
		public static final int GProM_T_CaseExpr = 13;
		/** <i>native declaration : line 312</i> */
		public static final int GProM_T_CaseWhen = 14;
		/** <i>native declaration : line 313</i> */
		public static final int GProM_T_IsNullExpr = 15;
		/** <i>native declaration : line 314</i> */
		public static final int GProM_T_WindowBound = 16;
		/** <i>native declaration : line 315</i> */
		public static final int GProM_T_WindowFrame = 17;
		/** <i>native declaration : line 316</i> */
		public static final int GProM_T_WindowDef = 18;
		/** <i>native declaration : line 317</i> */
		public static final int GProM_T_WindowFunction = 19;
		/** <i>native declaration : line 318</i> */
		public static final int GProM_T_RowNumExpr = 20;
		/** <i>native declaration : line 319</i> */
		public static final int GProM_T_OrderExpr = 21;
		/** <i>native declaration : line 320</i> */
		public static final int GProM_T_CastExpr = 22;
		/**
		 * query block model nodes<br>
		 * <i>native declaration : line 323</i>
		 */
		public static final int GProM_T_SetQuery = 23;
		/** <i>native declaration : line 324</i> */
		public static final int GProM_T_ProvenanceStmt = 24;
		/** <i>native declaration : line 325</i> */
		public static final int GProM_T_ProvenanceTransactionInfo = 25;
		/** <i>native declaration : line 326</i> */
		public static final int GProM_T_QueryBlock = 26;
		/** <i>native declaration : line 327</i> */
		public static final int GProM_T_SelectItem = 27;
		/** <i>native declaration : line 328</i> */
		public static final int GProM_T_FromItem = 28;
		/** <i>native declaration : line 329</i> */
		public static final int GProM_T_FromProvInfo = 29;
		/** <i>native declaration : line 330</i> */
		public static final int GProM_T_FromTableRef = 30;
		/** <i>native declaration : line 331</i> */
		public static final int GProM_T_FromSubquery = 31;
		/** <i>native declaration : line 332</i> */
		public static final int GProM_T_FromJoinExpr = 32;
		/** <i>native declaration : line 333</i> */
		public static final int GProM_T_DistinctClause = 33;
		/** <i>native declaration : line 334</i> */
		public static final int GProM_T_NestedSubquery = 34;
		/** <i>native declaration : line 335</i> */
		public static final int GProM_T_Insert = 35;
		/** <i>native declaration : line 336</i> */
		public static final int GProM_T_Delete = 36;
		/** <i>native declaration : line 337</i> */
		public static final int GProM_T_Update = 37;
		/** <i>native declaration : line 338</i> */
		public static final int GProM_T_TransactionStmt = 38;
		/** <i>native declaration : line 339</i> */
		public static final int GProM_T_WithStmt = 39;
		/** <i>native declaration : line 340</i> */
		public static final int GProM_T_DDLStatement = 40;
		/** <i>native declaration : line 341</i> */
		public static final int GProM_T_UtilityStatement = 41;
		/**
		 * query operator model nodes<br>
		 * <i>native declaration : line 344</i>
		 */
		public static final int GProM_T_Schema = 42;
		/** <i>native declaration : line 345</i> */
		public static final int GProM_T_AttributeDef = 43;
		/** <i>native declaration : line 346</i> */
		public static final int GProM_T_QueryOperator = 44;
		/** <i>native declaration : line 347</i> */
		public static final int GProM_T_SelectionOperator = 45;
		/** <i>native declaration : line 348</i> */
		public static final int GProM_T_ProjectionOperator = 46;
		/** <i>native declaration : line 349</i> */
		public static final int GProM_T_JoinOperator = 47;
		/** <i>native declaration : line 350</i> */
		public static final int GProM_T_AggregationOperator = 48;
		/** <i>native declaration : line 351</i> */
		public static final int GProM_T_ProvenanceComputation = 49;
		/** <i>native declaration : line 352</i> */
		public static final int GProM_T_TableAccessOperator = 50;
		/** <i>native declaration : line 353</i> */
		public static final int GProM_T_SetOperator = 51;
		/** <i>native declaration : line 354</i> */
		public static final int GProM_T_DuplicateRemoval = 52;
		/** <i>native declaration : line 355</i> */
		public static final int GProM_T_ConstRelOperator = 53;
		/** <i>native declaration : line 356</i> */
		public static final int GProM_T_NestingOperator = 54;
		/** <i>native declaration : line 357</i> */
		public static final int GProM_T_WindowOperator = 55;
		/** <i>native declaration : line 358</i> */
		public static final int GProM_T_OrderOperator = 56;
		/**
		 * datalog model nodes<br>
		 * <i>native declaration : line 361</i>
		 */
		public static final int GProM_T_DLNode = 57;
		/** <i>native declaration : line 362</i> */
		public static final int GProM_T_DLAtom = 58;
		/** <i>native declaration : line 363</i> */
		public static final int GProM_T_DLVar = 59;
		/** <i>native declaration : line 364</i> */
		public static final int GProM_T_DLRule = 60;
		/** <i>native declaration : line 365</i> */
		public static final int GProM_T_DLProgram = 61;
		/** <i>native declaration : line 366</i> */
		public static final int GProM_T_DLComparison = 62;
		/** <i>native declaration : line 367</i> */
		public static final int GProM_T_DLDomain = 63;
		/**
		 * Json Table GProMNode<br>
		 * <i>native declaration : line 370</i>
		 */
		public static final int GProM_T_FromJsonTable = 64;
		/** <i>native declaration : line 371</i> */
		public static final int GProM_T_JsonTableOperator = 65;
		/** <i>native declaration : line 372</i> */
		public static final int GProM_T_JsonColInfoItem = 66;
		/** <i>native declaration : line 373</i> */
		public static final int GProM_T_JsonPath = 67;
		/**
		 * relation<br>
		 * <i>native declaration : line 376</i>
		 */
		public static final int GProM_T_Relation = 68;
		/**
		 * rpq<br>
		 * <i>native declaration : line 379</i>
		 */
		public static final int GProM_T_Regex = 69;
		/** <i>native declaration : line 380</i> */
		public static final int GProM_T_RPQQuery = 70;
		/**
		 * ddl<br>
		 * <i>native declaration : line 383</i>
		 */
		public static final int GProM_T_CreateTable = 71;
		/** <i>native declaration : line 384</i> */
		public static final int GProM_T_AlterTable = 72;
	};

	public static interface GProMDataType {
		/** <i>native declaration : line 388</i> */
		public static final int GProM_DT_INT = 0;
		/** <i>native declaration : line 389</i> */
		public static final int GProM_DT_LONG = 1;
		/** <i>native declaration : line 390</i> */
		public static final int GProM_DT_STRING = 2;
		/** <i>native declaration : line 391</i> */
		public static final int GProM_DT_FLOAT = 3;
		/** <i>native declaration : line 392</i> */
		public static final int GProM_DT_BOOL = 4;
		/** <i>native declaration : line 393</i> */
		public static final int GProM_DT_VARCHAR2 = 5;
	};

	public static interface GProMJoinType {
		/** <i>native declaration : line 397</i> */
		public static final int GProM_JOIN_INNER = 0;
		/** <i>native declaration : line 398</i> */
		public static final int GProM_JOIN_CROSS = 1;
		/** <i>native declaration : line 399</i> */
		public static final int GProM_JOIN_LEFT_OUTER = 2;
		/** <i>native declaration : line 400</i> */
		public static final int GProM_JOIN_RIGHT_OUTER = 3;
		/** <i>native declaration : line 401</i> */
		public static final int GProM_JOIN_FULL_OUTER = 4;
	};

	public static interface GProMSetOpType {
		/** <i>native declaration : line 405</i> */
		public static final int GProM_SETOP_UNION = 0;
		/** <i>native declaration : line 406</i> */
		public static final int GProM_SETOP_INTERSECTION = 1;
		/** <i>native declaration : line 407</i> */
		public static final int GProM_SETOP_DIFFERENCE = 2;
	};

	public static interface GProMProvenanceType {
		/** <i>native declaration : line 411</i> */
		public static final int GProM_PROV_PI_CS = 0;
		/** <i>native declaration : line 412</i> */
		public static final int GProM_PROV_TRANSFORMATION = 1;
		/** <i>native declaration : line 413</i> */
		public static final int GProM_PROV_NONE = 2;
	};

	public static interface GProMProvenanceInputType {
		/** <i>native declaration : line 417</i> */
		public static final int GProM_PROV_INPUT_QUERY = 0;
		/** <i>native declaration : line 418</i> */
		public static final int GProM_PROV_INPUT_UPDATE = 1;
		/** <i>native declaration : line 419</i> */
		public static final int GProM_PROV_INPUT_UPDATE_SEQUENCE = 2;
		/** <i>native declaration : line 420</i> */
		public static final int GProM_PROV_INPUT_REENACT = 3;
		/** <i>native declaration : line 421</i> */
		public static final int GProM_PROV_INPUT_REENACT_WITH_TIMES = 4;
		/** <i>native declaration : line 422</i> */
		public static final int GProM_PROV_INPUT_TRANSACTION = 5;
		/** <i>native declaration : line 423</i> */
		public static final int GProM_PROV_INPUT_TIME_INTERVAL = 6;
	};

	public static interface GProMIsolationLevel {
		/** <i>native declaration : line 427</i> */
		public static final int GProM_ISOLATION_SERIALIZABLE = 0;
		/** <i>native declaration : line 428</i> */
		public static final int GProM_ISOLATION_READ_COMMITTED = 1;
		/** <i>native declaration : line 429</i> */
		public static final int GProM_ISOLATION_READ_ONLY = 2;
	};

	public static interface GProMNestingExprType {
		/** <i>native declaration : line 433</i> */
		public static final int GProM_NESTQ_EXISTS = 0;
		/** <i>native declaration : line 434</i> */
		public static final int GProM_NESTQ_ANY = 1;
		/** <i>native declaration : line 435</i> */
		public static final int GProM_NESTQ_ALL = 2;
		/** <i>native declaration : line 436</i> */
		public static final int GProM_NESTQ_UNIQUE = 3;
		/** <i>native declaration : line 437</i> */
		public static final int GProM_NESTQ_SCALAR = 4;
	};

	public static interface GProMWindowBoundType {
		/** <i>native declaration : line 441</i> */
		public static final int GProM_WINBOUND_UNBOUND_PREC = 0;
		/** <i>native declaration : line 442</i> */
		public static final int GProM_WINBOUND_CURRENT_ROW = 1;
		/** <i>native declaration : line 443</i> */
		public static final int GProM_WINBOUND_EXPR_PREC = 2;
		/** <i>native declaration : line 444</i> */
		public static final int GProM_WINBOUND_EXPR_FOLLOW = 3;
	};

	public static interface GProMWinFrameType {
		/** <i>native declaration : line 448</i> */
		public static final int GProM_WINFRAME_ROWS = 0;
		/** <i>native declaration : line 449</i> */
		public static final int GProM_WINFRAME_RANGE = 1;
	};

	public static interface GProMSortOrder {
		/** <i>native declaration : line 453</i> */
		public static final int GProM_SORT_ASC = 0;
		/** <i>native declaration : line 454</i> */
		public static final int GProM_SORT_DESC = 1;
	};

	public static interface GProMSortNullOrder {
		/** <i>native declaration : line 458</i> */
		public static final int GProM_SORT_NULLS_FIRST = 0;
		/** <i>native declaration : line 459</i> */
		public static final int GProM_SORT_NULLS_LAST = 1;
	};
	
	public static final int GProM_INVALID_PARAM = (int)-1;
	public static final int GProM_INVALID_ATTR = (int)-1;
	public static final int GProM_INVALID_FROM_ITEM = (int)-1;
	
	public Pointer gprom_rewriteQueryToOperatorModel (String query);
    
	public Pointer gprom_provRewriteOperator(Pointer nodeFromMimir);

	public Pointer gprom_nodeToString(Pointer nodeFromMimir);
	
	public Pointer gprom_OperatorModelToQuery(Pointer nodeFromMimir);
	
	public Pointer gprom_optimizeOperatorModel(Pointer nodeFromMimir);
	
	public Pointer gprom_operatorModelToSql(Pointer nodeFromMimir);
	
	public Pointer gprom_createMemContext();
	public Pointer gprom_createMemContextName(String ctxName);
	public void gprom_freeMemContext(Pointer memContext);
	public GProMHashMap gprom_addToMap(Pointer hashmap, Pointer key, Pointer value);

}
