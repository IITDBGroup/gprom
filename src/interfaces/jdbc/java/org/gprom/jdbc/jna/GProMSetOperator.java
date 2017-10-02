package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMSetOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/**
	 * @see GProMSetOpType<br>
	 * C type : GProMSetOpType
	 */
	public int setOpType;
	public GProMSetOperator() {
		super();
	}
	public GProMSetOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "setOpType");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param setOpType @see GProMSetOpType<br>
	 * C type : GProMSetOpType
	 */
	public GProMSetOperator(GProMQueryOperator op, int setOpType) {
		super();
		this.op = op;
		this.setOpType = setOpType;
		write();
	}
	public static class ByReference extends GProMSetOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMSetOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, int setOpType){
			super(op,setOpType);
		}
		
	};
}
