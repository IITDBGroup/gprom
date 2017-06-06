package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMCaseExpr extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
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
	public GProMCaseExpr(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "expr", "whenClauses", "elseRes");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
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
		write();
	}
	public static class ByReference extends GProMCaseExpr implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMCaseExpr implements Structure.ByValue {
		public ByValue(int type, org.gprom.jdbc.jna.GProMNode.ByReference expr, org.gprom.jdbc.jna.GProMList.ByReference whenClauses, org.gprom.jdbc.jna.GProMNode.ByReference elseRes){
			super(type,expr,whenClauses,elseRes);
		}
		
	};
}
