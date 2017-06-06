package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;


public class GProMFunctionCall extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : String */
	public String functionname;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference args;
	public int isAgg;
	public GProMFunctionCall() {
		super();
	}
	public GProMFunctionCall(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "functionname", "args", "isAgg");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param functionname C type : String<br>
	 * @param args C type : GProMList*
	 */
	public GProMFunctionCall(int type, String functionname, org.gprom.jdbc.jna.GProMList.ByReference args, int isAgg) {
		super();
		this.type = type;
		this.functionname = functionname;
		this.args = args;
		this.isAgg = isAgg;
		write();
	}
	public static class ByReference extends GProMFunctionCall implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMFunctionCall implements Structure.ByValue {
		public ByValue(int type, String functionname, org.gprom.jdbc.jna.GProMList.ByReference args, int isAgg){
			super(type,functionname,args,isAgg);
		}
		
	};
}
