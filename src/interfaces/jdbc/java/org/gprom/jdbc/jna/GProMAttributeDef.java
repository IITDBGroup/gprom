package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
public class GProMAttributeDef extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/**
	 * @see GProMDataType<br>
	 * C type : GProMDataType
	 */
	public int dataType;
	/** C type : char* */
	public String attrName;
	public GProMAttributeDef() {
		super();
	}
	public GProMAttributeDef(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "dataType", "attrName");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param dataType @see GProMDataType<br>
	 * C type : GProMDataType<br>
	 * @param attrName C type : char*
	 */
	public GProMAttributeDef(int type, int dataType, String attrName) {
		super();
		this.type = type;
		this.dataType = dataType;
		this.attrName = attrName;
	}
	public static class ByReference extends GProMAttributeDef implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMAttributeDef implements Structure.ByValue {
		
	};
}
