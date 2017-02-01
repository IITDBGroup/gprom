package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMCaseExpr extends GProMStructure {
	public int type;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference expr;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference whenClauses;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference elseRes;
	public GProMCaseExpr() {
		super();
	}
	public GProMCaseExpr(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "expr", "whenClauses", "elseRes");
	}
	/**
	 * @param expr C type : GProMNode*<br>
	 * @param whenClauses C type : GProMList*<br>
	 * @param elseRes C type : GProMNode*
	 */
	public GProMCaseExpr(int type, org.gprom.jdbc.jna.GProMNode.ByReference expr, org.gprom.jdbc.jna.GProMList.ByReference whenClauses, org.gprom.jdbc.jna.GProMNode.ByReference elseRes) {
		super();
		this.type = type;
		this.expr = expr;
		this.whenClauses = whenClauses;
		this.elseRes = elseRes;
	}
	public static class ByReference extends GProMCaseExpr implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMCaseExpr implements Structure.ByValue {
		
	};
}
