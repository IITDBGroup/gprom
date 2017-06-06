package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMConstRelOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference values;
	public GProMConstRelOperator() {
		super();
	}
	public GProMConstRelOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "values");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param values C type : GProMList*
	 */
	public GProMConstRelOperator(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference values) {
		super();
		this.op = op;
		this.values = values;
		write();
	}
	public static class ByReference extends GProMConstRelOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMConstRelOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference values){
			super(op,values);
		}
		
	};
}
