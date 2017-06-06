package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMSelectionOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/**
	 * condition expression<br>
	 * C type : GProMNode*
	 */
	public org.gprom.jdbc.jna.GProMNode.ByReference cond;
	public GProMSelectionOperator() {
		super();
	}
	public GProMSelectionOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "cond");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param cond condition expression<br>
	 * C type : GProMNode*
	 */
	public GProMSelectionOperator(GProMQueryOperator op, org.gprom.jdbc.jna.GProMNode.ByReference cond) {
		super();
		this.op = op;
		this.cond = cond;
		write();
	}
	public static class ByReference extends GProMSelectionOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMSelectionOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, org.gprom.jdbc.jna.GProMNode.ByReference cond){
			super(op,cond);
		}
		
	};
}
