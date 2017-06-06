package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;


public class GProMOperator extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : String */
	public String name;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference args;
	public GProMOperator() {
		super();
	}
	public GProMOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "name", "args");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param name C type : String<br>
	 * @param args C type : GProMList*
	 */
	public GProMOperator(int type, String name, org.gprom.jdbc.jna.GProMList.ByReference args) {
		super();
		this.type = type;
		this.name = name;
		this.args = args;
		write();
	}
	public static class ByReference extends GProMOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMOperator implements Structure.ByValue {
		public ByValue(int type, String name, org.gprom.jdbc.jna.GProMList.ByReference args){
			super(type,name,args);
		}
		
	};
}
