package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;


public class GProMUpdateOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/** C type : String */
	public String tableName;
	public GProMUpdateOperator() {
		super();
	}
	public GProMUpdateOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "tableName");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param tableName C type : String
	 */
	public GProMUpdateOperator(GProMQueryOperator op, String tableName) {
		super();
		this.op = op;
		this.tableName = tableName;
		write();
	}
	public static class ByReference extends GProMUpdateOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMUpdateOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, String tableName){
			super(op,tableName);
		}
		
	};
}
