package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMKeyValue extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference key;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference value;
	public GProMKeyValue() {
		super();
	}
	public GProMKeyValue(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "key", "value");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param key C type : GProMNode*<br>
	 * @param value C type : GProMNode*
	 */
	public GProMKeyValue(int type, org.gprom.jdbc.jna.GProMNode.ByReference key, org.gprom.jdbc.jna.GProMNode.ByReference value) {
		super();
		this.type = type;
		this.key = key;
		this.value = value;
		write();
	}
	public static class ByReference extends GProMKeyValue implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMKeyValue implements Structure.ByValue {
		public ByValue(int type, org.gprom.jdbc.jna.GProMNode.ByReference key, org.gprom.jdbc.jna.GProMNode.ByReference value){
			super(type,key,value);
		}
		
	};
}
