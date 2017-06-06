package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMWindowBound extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/**
	 * @see GProMWindowBoundType<br>
	 * C type : GProMWindowBoundType
	 */
	public int bType;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference expr;
	public GProMWindowBound() {
		super();
	}
	public GProMWindowBound(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "bType", "expr");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param bType @see GProMWindowBoundType<br>
	 * C type : GProMWindowBoundType<br>
	 * @param expr C type : GProMNode*
	 */
	public GProMWindowBound(int type, int bType, org.gprom.jdbc.jna.GProMNode.ByReference expr) {
		super();
		this.type = type;
		this.bType = bType;
		this.expr = expr;
		write();
	}
	public static class ByReference extends GProMWindowBound implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMWindowBound implements Structure.ByValue {
		public ByValue(int type, int bType, org.gprom.jdbc.jna.GProMNode.ByReference expr){
			super(type,bType,expr);
		}
		
	};
}
