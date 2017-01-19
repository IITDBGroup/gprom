package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
public class GProMSchema extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : char* */
	public String name;
	/**
	 * AttributeDef type<br>
	 * C type : GProMList*
	 */
	public org.gprom.jdbc.jna.GProMList.ByReference attrDefs;
	public GProMSchema() {
		super();
	}
	public GProMSchema(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "name", "attrDefs");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param name C type : char*<br>
	 * @param attrDefs AttributeDef type<br>
	 * C type : GProMList*
	 */
	public GProMSchema(int type, String name, org.gprom.jdbc.jna.GProMList.ByReference attrDefs) {
		super();
		this.type = type;
		this.name = name;
		this.attrDefs = attrDefs;
	}
	public static class ByReference extends GProMSchema implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMSchema implements Structure.ByValue {
		
	};
}
