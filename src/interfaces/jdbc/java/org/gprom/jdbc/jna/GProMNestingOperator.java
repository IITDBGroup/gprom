package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMNestingOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/**
	 * @see GProMNestingExprType<br>
	 * C type : GProMNestingExprType
	 */
	public int nestingType;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference cond;
	public GProMNestingOperator() {
		super();
	}
	public GProMNestingOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "nestingType", "cond");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param nestingType @see GProMNestingExprType<br>
	 * C type : GProMNestingExprType<br>
	 * @param cond C type : GProMNode*
	 */
	public GProMNestingOperator(GProMQueryOperator op, int nestingType, org.gprom.jdbc.jna.GProMNode.ByReference cond) {
		super();
		this.op = op;
		this.nestingType = nestingType;
		this.cond = cond;
		write();
	}
	public static class ByReference extends GProMNestingOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMNestingOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, int nestingType, org.gprom.jdbc.jna.GProMNode.ByReference cond){
			super(op,nestingType,cond);
		}
		
	};
}
