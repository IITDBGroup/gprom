package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMJoinOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/**
	 * @see GProMJoinType<br>
	 * C type : GProMJoinType
	 */
	public int joinType;
	/**
	 * join condition expression<br>
	 * C type : GProMNode*
	 */
	public org.gprom.jdbc.jna.GProMNode.ByReference cond;
	public GProMJoinOperator() {
		super();
	}
	public GProMJoinOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "joinType", "cond");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param joinType @see GProMJoinType<br>
	 * C type : GProMJoinType<br>
	 * @param cond join condition expression<br>
	 * C type : GProMNode*
	 */
	public GProMJoinOperator(GProMQueryOperator op, int joinType, org.gprom.jdbc.jna.GProMNode.ByReference cond) {
		super();
		this.op = op;
		this.joinType = joinType;
		this.cond = cond;
		write();
	}
	public static class ByReference extends GProMJoinOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMJoinOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, int joinType, org.gprom.jdbc.jna.GProMNode.ByReference cond){
			super(op,joinType,cond);
		}
		
	};
}
