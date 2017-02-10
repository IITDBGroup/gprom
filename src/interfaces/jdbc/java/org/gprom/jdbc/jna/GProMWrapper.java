/**
 * 
 */
package org.gprom.jdbc.jna;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.gprom.jdbc.utility.PropertyWrapper;

import com.sun.jna.Pointer;

/**
 * @author lord_pretzel
 *
 */
public class GProMWrapper implements GProMJavaInterface {

	static Logger libLog = Logger.getLogger("LIBGPROM");
	static Logger log = Logger.getLogger(GProMWrapper.class);

	public static final String KEY_CONNECTION_HOST = "connection.host";
	public static final String KEY_CONNECTION_DATABASE = "connection.db";
	public static final String KEY_CONNECTION_USER = "connection.user";
	public static final String KEY_CONNECTION_PASSWORD = "connection.passwd";
	public static final String KEY_CONNECTION_PORT = "connection.port";
	
	private class ExceptionInfo {
	
		public String message;
		public String file;
		public int line;
		public ExceptionSeverity severity;
		
		public ExceptionInfo (String message, String file, int line, ExceptionSeverity severity) {
			this.message = message;
			this.file = file;
			this.line = line;
			this.severity = severity;
		}
		
		public String toString() {
			return severity.toString() + ":" + file + "-" + line + " " + message ;
		}
		
	}
	
	private GProM_JNA.GProMLoggerCallbackFunction loggerCallback;
	private GProM_JNA.GProMExceptionCallbackFunction exceptionCallback;
	private List<ExceptionInfo> exceptions;
	private GProMMetadataLookupPlugin p;
	
	// singleton instance	
	public static GProMWrapper inst = new GProMWrapper ();

	public static GProMWrapper getInstance () {
		return inst; 
	}

	private boolean silenceLogger = false;
	public void setSilenceLogger(boolean silenceLogger){
		this.silenceLogger = silenceLogger;
		if(silenceLogger){
			libLog.setLevel(Level.OFF);
			log.setLevel(Level.OFF);
		}
		else{
			Level ll = intToLogLevel( GProM_JNA.INSTANCE.gprom_getIntOption("log.level"));
			libLog.setLevel(ll);
			log.setLevel(ll);
		}
	}
	
	private GProMWrapper () {
		exceptions = new ArrayList<ExceptionInfo> ();
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#gpromRewriteQuery(java.lang.String)
	 */
	@Override
	public String gpromRewriteQuery(String query) throws SQLException {
		Pointer p =  GProM_JNA.INSTANCE.gprom_rewriteQuery(query);
		String result = p.getString(0);
		
		// check whether exception has occured
		if (exceptions.size() > 0) {
			StringBuilder mes = new StringBuilder();
			for(ExceptionInfo i: exceptions)
			{
				mes.append("ERROR (" + i + ")");
				mes.append(i.toString());
				mes.append("\n\n");
			}
			exceptions.clear();
			log.error("have encountered exception");
			throw new NativeGProMLibException("Error during rewrite:\n" + mes.toString());
		}
		//TODO use string builder to avoid creation of two large strings
		result = result.replaceFirst(";\\s+\\z", "");
		log.info("HAVE REWRITTEN:\n\n" + query + "\n\ninto:\n\n" + result);
		return result;
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#rewriteQueryToOperatorModel(java.lang.String)
	 */
	@Override
	public GProMStructure rewriteQueryToOperatorModel(String query) throws Exception {
		Pointer p =  GProM_JNA.INSTANCE.gprom_rewriteQueryToOperatorModel(query);
		GProMStructure result;
		
		GProMNode gpromNode = new GProMNode(p);
		result = castGProMNode(gpromNode);
		
		// check whether exception has occured
		if (exceptions.size() > 0) {
			StringBuilder mes = new StringBuilder();
			for(ExceptionInfo i: exceptions)
			{
				mes.append("ERROR (" + i + ")");
				mes.append(i.toString());
				mes.append("\n\n");
			}
			exceptions.clear();
			log.error("have encountered exception");
			throw new NativeGProMLibException("Error during rewrite:\n" + mes.toString());
		}
		
		return result;
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#provRewriteOperator(com.sun.jna.Pointer)
	 */
	@Override
	public GProMStructure provRewriteOperator(Pointer nodeFromMimir) throws Exception {
		Pointer p =  GProM_JNA.INSTANCE.gprom_provRewriteOperator(nodeFromMimir);
		GProMStructure result;
		
		GProMNode gpromNode = new GProMNode(p);
		result = castGProMNode(gpromNode);
		
		// check whether exception has occured
		if (exceptions.size() > 0) {
			StringBuilder mes = new StringBuilder();
			for(ExceptionInfo i: exceptions)
			{
				mes.append("ERROR (" + i + ")");
				mes.append(i.toString());
				mes.append("\n\n");
			}
			exceptions.clear();
			log.error("have encountered exception");
			throw new NativeGProMLibException("Error during rewrite:\n" + mes.toString());
		}
		
		return result;
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#gpromNodeToString(com.sun.jna.Pointer)
	 */
	@Override
	public String gpromNodeToString(Pointer nodeFromMimir) throws Exception {
		Pointer p =  GProM_JNA.INSTANCE.gprom_nodeToString(nodeFromMimir);
		String result = p.getString(0);
		
		// check whether exception has occured
		if (exceptions.size() > 0) {
			StringBuilder mes = new StringBuilder();
			for(ExceptionInfo i: exceptions)
			{
				mes.append("ERROR (" + i + ")");
				mes.append(i.toString());
				mes.append("\n\n");
			}
			exceptions.clear();
			log.error("have encountered exception");
			throw new NativeGProMLibException("Error during rewrite:\n" + mes.toString());
		}
		//TODO use string builder to avoid creation of two large strings
		result = result.replaceFirst(";\\s+\\z", "");
		//log.info("NodeToString:\n\n" + result );
		return result;
	}
	
	@Override
	public void gpromCreateMemContext(){
		GProM_JNA.INSTANCE.gprom_createMemContext();
	}
	
	@Override
	public void gpromFreeMemContext(){
		GProM_JNA.INSTANCE.gprom_freeMemContext();
	}
	
	public GProMStructure castGProMNode(GProMNode node){
		int nodeType = node.type;
		
		switch(nodeType)
	    {
	        /* collection type nodes*/
	        case GProM_JNA.GProMNodeTag.GProM_T_List:
	        case GProM_JNA.GProMNodeTag.GProM_T_IntList:
	            return new GProMList(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Set:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_HashMap:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Vector:
	            return new GProMNode(node.getPointer());

	        /* expression model */
	        case GProM_JNA.GProMNodeTag.GProM_T_AttributeReference:
	            return new GProMAttributeReference(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_FunctionCall:
	            return new GProMFunctionCall(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_KeyValue:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Operator:
	            return new GProMOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Schema:
	            return new GProMSchema(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_AttributeDef:
	            return new GProMAttributeDef(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_SQLParameter:
	            return new GProMSQLParameter(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_CaseExpr:
	            return new GProMCaseExpr(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_CaseWhen:
	            return new GProMCaseWhen(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_IsNullExpr:
	            return new GProMIsNullExpr(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_WindowBound:
	            return new GProMWindowBound(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_WindowFrame:
	            return new GProMWindowFrame(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_WindowDef:
	            return new GProMWindowDef(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_WindowFunction:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_RowNumExpr:
	            return new GProMRowNumExpr(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_OrderExpr:
	            return new GProMOrderExpr(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_CastExpr:
	            return new GProMCastExpr(node.getPointer());
	        /* query block model nodes */
//	        case GProM_JNA.GProMNodeTag.GProM_T_SetOp:
//	            return new GProMNode(node.getPointer());
//	        case GProM_JNA.GProMNodeTag.GProM_T_SetQuery:
//	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_ProvenanceStmt:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_ProvenanceTransactionInfo:
	            return new GProMProvenanceTransactionInfo(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_QueryBlock:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_WithStmt:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_SelectItem:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Constant:
	            return new GProMConstant(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_NestedSubquery:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_FromTableRef:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_FromSubquery:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_FromJoinExpr:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_FromProvInfo:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_DistinctClause:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Insert:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Delete:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Update:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_TransactionStmt:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_CreateTable:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_AlterTable:
	            return new GProMNode(node.getPointer());
	        /* query operator model nodes */
	        case GProM_JNA.GProMNodeTag.GProM_T_SelectionOperator:
	            return new GProMSelectionOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_ProjectionOperator:
	            return new GProMProjectionOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_JoinOperator:
	            return new GProMJoinOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_AggregationOperator:
	            return new GProMAggregationOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_ProvenanceComputation:
	            return new GProMProvenanceComputation(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_TableAccessOperator:
	            return new GProMTableAccessOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_SetOperator:
	            return new GProMSetOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_DuplicateRemoval:
	            return new GProMDuplicateRemoval(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_ConstRelOperator:
	            return new GProMConstRelOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_NestingOperator:
	            return new GProMNestingOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_WindowOperator:
	            return new GProMWindowOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_OrderOperator:
	            return new GProMOrderOperator(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_FromJsonTable:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_JsonColInfoItem:
		    return new GProMNode(node.getPointer());
		    /* datalog model nodes */
	        case GProM_JNA.GProMNodeTag.GProM_T_DLAtom:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_DLVar:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_DLRule:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_DLProgram:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_DLComparison:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_DLDomain:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_JsonTableOperator:
	        	return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_JsonPath:
	        	return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_Regex:
	            return new GProMNode(node.getPointer());
	        case GProM_JNA.GProMNodeTag.GProM_T_RPQQuery:
	            return new GProMNode(node.getPointer());
	        default:
	            return null;
	            
	    }
	}

	public void init () {
		loggerCallback = new GProM_JNA.GProMLoggerCallbackFunction () {
			public void invoke(String message, String file, int line, int level) {
				logCallbackFunction(message, file, line, level);
			}
		};
		
		exceptionCallback = new GProM_JNA.GProMExceptionCallbackFunction() {
			
			@Override
			public int invoke(String message, String file, int line, int severity) {
				return exceptionCallbackFunction(message, file, line, severity);
			}

		};

		GProM_JNA.INSTANCE.gprom_registerLoggerCallbackFunction(loggerCallback);
		GProM_JNA.INSTANCE.gprom_registerExceptionCallbackFunction(exceptionCallback);
		GProM_JNA.INSTANCE.gprom_init();
		GProM_JNA.INSTANCE.gprom_setMaxLogLevel(4);
	}

	public void setLogLevel (int level)
	{
		GProM_JNA.INSTANCE.gprom_setBoolOption("log.active", true);
		GProM_JNA.INSTANCE.gprom_setIntOption("log.level", level);
		GProM_JNA.INSTANCE.gprom_setMaxLogLevel(level);
	}

	public void setupOptions (String[] opts)
	{
		GProM_JNA.INSTANCE.gprom_readOptions(opts.length, opts);
	}

	public void setupPlugins ()
	{
		GProM_JNA.INSTANCE.gprom_configFromOptions();
	}
	
	public void reconfPlugins()
	{
		GProM_JNA.INSTANCE.gprom_reconfPlugins();
	}
	
	public void setupPlugins(Connection con, GProMMetadataLookupPlugin p)
	{
		setStringOption("plugin.metadata", "external");
		GProM_JNA.INSTANCE.gprom_configFromOptions();
		this.p = p;
		GProM_JNA.INSTANCE.gprom_registerMetadataLookupPlugin(p);
	}

	public void setupFromOptions (String[] opts)
	{
		setupOptions(opts);
		setupPlugins();
	}

	public void setupFromOptions (PropertyWrapper options) throws Exception {
		setupOptions(options);
		setupPlugins();
	}
	
	public void shutdown()
	{
		GProM_JNA.INSTANCE.gprom_shutdown();
	}

	private void logCallbackFunction (String message, String file, int line, int level) {
		String printMes = file + " at " + line + ": " + message;
		libLog.log(intToLogLevel(level), printMes);
	}

	private int exceptionCallbackFunction(String message, String file,
			int line, int severity) {
		String printMes = "EXCEPTION: " + file + " at " + line + ": " + message;
		libLog.error(printMes);
		exceptions.add(new ExceptionInfo(message, file, line, intToSeverity(severity)));
		return exceptionHandlerToInt(ExceptionHandler.Abort);
	}

	
	public Level intToLogLevel (int in)
	{
		if (in == 0 || in == 1)
			return Level.FATAL;
		if (in == 2)
			return Level.ERROR;
		if (in == 3)
			return Level.INFO;
		if (in == 4)
			return Level.DEBUG;

		return Level.DEBUG;
	}

	public void setOption (String key, String value) throws NumberFormatException, Exception {
		switch(typeOfOption(key)) {
		case Boolean:
			setBoolOption(key, Boolean.getBoolean(value));
			break;
		case Float:
			setFloatOption(key, Float.valueOf(value));
			break;
		case Int:
			setIntOption(key, Integer.valueOf(value));
			break;
		case String:
			setStringOption(key, value);
			break;
		default:
			break;
		}
	}
	
	public String getOption (String key) throws NumberFormatException, Exception {
		switch(typeOfOption(key)) {
		case Boolean:
			return "" + getBoolOption(key);
		case Float:
			return "" + getFloatOption(key);
		case Int:
			return "" + getIntOption(key);
		case String:
			return getStringOption(key);
		default:
			break;
		}
		throw new Exception ("unkown option " + key);
	}
	
	public String getStringOption (String name)
	{
		return GProM_JNA.INSTANCE.gprom_getStringOption(name);
	}

	public int getIntOption (String name)
	{
		return GProM_JNA.INSTANCE.gprom_getIntOption(name);	
	}

	public boolean getBoolOption (String name)
	{
		return GProM_JNA.INSTANCE.gprom_getBoolOption(name);
	}

	public double getFloatOption (String name)
	{
		return GProM_JNA.INSTANCE.gprom_getFloatOption(name);
	}

	public void setStringOption (String name, String value)
	{
		GProM_JNA.INSTANCE.gprom_setStringOption(name, value);
	}

	public void setIntOption(String name, int value)
	{
		GProM_JNA.INSTANCE.gprom_setIntOption(name, value);
	}

	public void setBoolOption(String name, boolean value)
	{
		GProM_JNA.INSTANCE.gprom_setBoolOption(name, value);
	}

	public void setFloatOption(String name, double value)
	{
		GProM_JNA.INSTANCE.gprom_setFloatOption(name, value);
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#optionExists(java.lang.String)
	 */
	@Override
	public boolean optionExists(String name) {
		// TODO Auto-generated method stub
		return GProM_JNA.INSTANCE.gprom_optionExists(name);
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#typeOfOption(java.lang.String)
	 */
	@Override
	public OptionType typeOfOption(String name) throws Exception {
		if (GProM_JNA.INSTANCE.gprom_optionExists(name))
		{
			String optionType = GProM_JNA.INSTANCE.gprom_getOptionType(name);
			if(optionType.equals("OPTION_STRING"))
				return OptionType.String;
			if(optionType.equals("OPTION_BOOL"))
				return OptionType.Boolean;
			if(optionType.equals("OPTION_FLOAT"))
				return OptionType.Float;
			if(optionType.equals("OPTION_INT"))
				return OptionType.Int;
		}
		throw new Exception("option " + name + " does is not a valid option");
	}

	/** 
	 * @see org.gprom.jdbc.jna.GProMJavaInterface#setOptions(java.util.Properties)
	 */
	@Override
	public void setupOptions(PropertyWrapper options) throws Exception {
		for (String key: options.stringPropertyNames()) {
			log.debug("key: "+ key + " type: " + typeOfOption(key) + " value: " + options.getString(key));
			
			switch(typeOfOption(key)) {
			case Boolean:
				setBoolOption(key, options.getBool(key));
				break;
			case Float:
				setFloatOption(key, options.getFloat(key));
				break;
			case Int:
				setIntOption(key, options.getInt(key));
				break;
			case String:
				setStringOption(key, options.getString(key));
				break;
			default:
				break;
			}
		}
	}


	public void setConnectionOption (PropertyWrapper opts, ConnectionParam p, String value) {
		switch(p)
		{
		case Database:
			log.debug("set key " + KEY_CONNECTION_DATABASE + " to " + value);
			opts.setProperty(KEY_CONNECTION_DATABASE, value);
			break;
		case Host:
			log.debug("set key " + KEY_CONNECTION_HOST + " to " + value);
			opts.setProperty(KEY_CONNECTION_HOST, value);
			break;
		case Password:
			log.debug("set key " + KEY_CONNECTION_PASSWORD + " to " + value);
			opts.setProperty(KEY_CONNECTION_PASSWORD, value);
			break;
		case Port:
			log.debug("set key " + KEY_CONNECTION_PORT + " to " + value);
			opts.setProperty(KEY_CONNECTION_PORT, value);
			break;
		case User:
			log.debug("set key " + KEY_CONNECTION_USER + " to " + value);
			opts.setProperty(KEY_CONNECTION_USER, value);
			break;
		default:
			break;
		}
	}
	
	private int exceptionHandlerToInt (ExceptionHandler h) {
		switch(h) {
		case Abort:
			return 1;
		case Die:
			return 0;
		case Wipe:
			return 2;
		default:
			return 0;
		}
	}
	
	private ExceptionSeverity intToSeverity (int s) {
		if (s == 0)
			return ExceptionSeverity.Recoverable;
		if (s == 1)
			return ExceptionSeverity.Panic;
		return ExceptionSeverity.Panic;
	}

	public GProMMetadataLookupPlugin getP() {
		return p;
	}

	public void setP(GProMMetadataLookupPlugin p) {
		this.p = p;
	}
	
}
