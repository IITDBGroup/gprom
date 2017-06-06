package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMOrderOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference orderExprs;
	public GProMOrderOperator() {
		super();
	}
	public GProMOrderOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "orderExprs");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param orderExprs C type : GProMList*
	 */
	public GProMOrderOperator(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference orderExprs) {
		super();
		this.op = op;
		this.orderExprs = orderExprs;
		write();
	}
	public static class ByReference extends GProMOrderOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMOrderOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference orderExprs){
			super(op,orderExprs);
		}
		
	};
}
