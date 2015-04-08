/**
 * 
 */
package org.gprom.jdbc.jna;

import java.util.List;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.NativeLong;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
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
	public class GProMMetadataLookupPlugin extends Structure {
		/// C type : initMetadataLookupPlugin_callback
		public GProMMetadataLookupPlugin.initMetadataLookupPlugin_callback initMetadataLookupPlugin;
		/// C type : databaseConnectionOpen_callback
		public GProMMetadataLookupPlugin.databaseConnectionOpen_callback databaseConnectionOpen;
		/// C type : databaseConnectionClose_callback
		public GProMMetadataLookupPlugin.databaseConnectionClose_callback databaseConnectionClose;
		/// C type : shutdownMetadataLookupPlugin_callback
		public GProMMetadataLookupPlugin.shutdownMetadataLookupPlugin_callback shutdownMetadataLookupPlugin;
		/// C type : getAttributes_callback
		public GProMMetadataLookupPlugin.getAttributes_callback getAttributes;
		/// C type : getAttributeNames_callback
		public GProMMetadataLookupPlugin.getAttributeNames_callback getAttributeNames;
		/// C type : getAttributeDefaultVal_callback
		public GProMMetadataLookupPlugin.getAttributeDefaultVal_callback getAttributeDefaultVal;
		/// C type : getFuncReturnType_callback
		public GProMMetadataLookupPlugin.getFuncReturnType_callback getFuncReturnType;
		/// C type : getOpReturnType_callback
		public GProMMetadataLookupPlugin.getOpReturnType_callback getOpReturnType;
		/// C type : getTableDefinition_callback
		public GProMMetadataLookupPlugin.getTableDefinition_callback getTableDefinition;
		/// C type : getViewDefinition_callback
		public GProMMetadataLookupPlugin.getViewDefinition_callback getViewDefinition;
		public interface boolean_callback extends Callback {
			int apply();
		};
		public interface initMetadataLookupPlugin_callback extends Callback {
			int apply();
		};
		public interface databaseConnectionOpen_callback extends Callback {
			int apply();
		};
		public interface databaseConnectionClose_callback extends Callback {
			int apply();
		};
		public interface shutdownMetadataLookupPlugin_callback extends Callback {
			int apply();
		};
		public interface boolean_callback2 extends Callback {
			int apply(Pointer tableName);
		};
		public interface boolean_callback3 extends Callback {
			int apply(Pointer viewName);
		};
		public interface getAttributes_callback extends Callback {
			void apply(Pointer tableName, PointerByReference attrs, PointerByReference dataTypes, IntByReference numArgs);
		};
		public interface getAttributeNames_callback extends Callback {
			void apply(Pointer tableName, PointerByReference attrs, IntByReference numArgs);
		};
		public interface getAttributeDefaultVal_callback extends Callback {
			Pointer apply(Pointer schema, Pointer tableName, Pointer attrName);
		};
		public interface boolean_callback4 extends Callback {
			int apply(Pointer functionName);
		};
		public interface boolean_callback5 extends Callback {
			int apply(Pointer functionName);
		};
		public interface getFuncReturnType_callback extends Callback {
			Pointer apply(Pointer fName, PointerByReference args, int numArgs);
		};
		public interface getOpReturnType_callback extends Callback {
			Pointer apply(Pointer oName, PointerByReference args, int numArgs);
		};
		public interface getTableDefinition_callback extends Callback {
			Pointer apply(Pointer tableName);
		};
		public interface getViewDefinition_callback extends Callback {
			Pointer apply(Pointer viewName);
		};
		public GProMMetadataLookupPlugin() {
			super();
			initFieldOrder();
		}
		protected void initFieldOrder() {
			setFieldOrder(new String[]{"initMetadataLookupPlugin", "databaseConnectionOpen", "databaseConnectionClose", "shutdownMetadataLookupPlugin", "getAttributes", "getAttributeNames", "getAttributeDefaultVal", "getFuncReturnType", "getOpReturnType", "getTableDefinition", "getViewDefinition"});
		}
		protected ByReference newByReference() { return new ByReference(); }
		protected ByValue newByValue() { return new ByValue(); }
		protected GProMMetadataLookupPlugin newInstance() { return new GProMMetadataLookupPlugin(); }
//		public static GProMMetadataLookupPlugin[] newArray(int arrayLength) {
//			return Structure.newArray(GProMMetadataLookupPlugin.class, arrayLength);
//		}
		public static class ByReference extends GProMMetadataLookupPlugin implements Structure.ByReference {
			
		};
		public static class ByValue extends GProMMetadataLookupPlugin implements Structure.ByValue {
			
		}
		/* (non-Javadoc)
		 * @see com.sun.jna.Structure#getFieldOrder()
		 */
		@Override
		protected List getFieldOrder() {
			// TODO Auto-generated method stub
			return null;
		};
	}

	void gprom_registerMetadataLookupPlugin(GProMMetadataLookupPlugin plugin);
}
