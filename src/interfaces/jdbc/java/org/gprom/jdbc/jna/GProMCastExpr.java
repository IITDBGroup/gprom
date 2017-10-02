package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMCastExpr extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/**
	 * @see GProMDataType<br>
	 * C type : GProMDataType
	 */
	public int resultDT;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference expr;
	public GProMCastExpr() {
		super();
	}
	public GProMCastExpr(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "resultDT", "expr");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param resultDT @see GProMDataType<br>
	 * C type : GProMDataType<br>
	 * @param expr C type : GProMNode*
	 */
	public GProMCastExpr(int type, int resultDT, org.gprom.jdbc.jna.GProMNode.ByReference expr) {
		super();
		this.type = type;
		this.resultDT = resultDT;
		this.expr = expr;
		write();
	}
	public static class ByReference extends GProMCastExpr implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMCastExpr implements Structure.ByValue {
		public ByValue(int type, int resultDT, org.gprom.jdbc.jna.GProMNode.ByReference expr){
			super(type,resultDT,expr);
		}
		
	};
}
