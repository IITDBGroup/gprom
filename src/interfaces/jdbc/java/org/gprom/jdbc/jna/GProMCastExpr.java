package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMCastExpr extends GProMStructure {
	public int type;
	/** C type : GProMDataType */
	public int resultDT;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference expr;
	public GProMCastExpr() {
		super();
	}
	public GProMCastExpr(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "resultDT", "expr");
	}
	/**
	 * @param resultDT C type : GProMDataType<br>
	 * @param expr C type : GProMNode*
	 */
	public GProMCastExpr(int type, int resultDT, org.gprom.jdbc.jna.GProMNode.ByReference expr) {
		super();
		this.type = type;
		this.resultDT = resultDT;
		this.expr = expr;
	}
	public static class ByReference extends GProMCastExpr implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMCastExpr implements Structure.ByValue {
		
	};
}
