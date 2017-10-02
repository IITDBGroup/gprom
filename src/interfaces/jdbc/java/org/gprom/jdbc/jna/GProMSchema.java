package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;


public class GProMSchema extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : String */
	public String name;
	/**
	 * GProMAttributeDef type<br>
	 * C type : GProMList*
	 */
	public org.gprom.jdbc.jna.GProMList.ByReference attrDefs;
	public GProMSchema() {
		super();
	}
	public GProMSchema(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "name", "attrDefs");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param name C type : String<br>
	 * @param attrDefs GProMAttributeDef type<br>
	 * C type : GProMList*
	 */
	public GProMSchema(int type, String name, org.gprom.jdbc.jna.GProMList.ByReference attrDefs) {
		super();
		this.type = type;
		this.name = name;
		this.attrDefs = attrDefs;
		write();
	}
	public static class ByReference extends GProMSchema implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMSchema implements Structure.ByValue {
		public ByValue(int type, String name, org.gprom.jdbc.jna.GProMList.ByReference attrDefs){
			super(type,name,attrDefs);
		}
		
	};
}
