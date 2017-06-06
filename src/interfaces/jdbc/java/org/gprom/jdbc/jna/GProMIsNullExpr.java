package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMIsNullExpr extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference expr;
	public GProMIsNullExpr() {
		super();
	}
	public GProMIsNullExpr(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "expr");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param expr C type : GProMNode*
	 */
	public GProMIsNullExpr(int type, org.gprom.jdbc.jna.GProMNode.ByReference expr) {
		super();
		this.type = type;
		this.expr = expr;
		write();
	}
	public static class ByReference extends GProMIsNullExpr implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMIsNullExpr implements Structure.ByValue {
		public ByValue(int type, org.gprom.jdbc.jna.GProMNode.ByReference expr){
			super(type,expr);
		}
		
	};
}
