package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
public class GProMUpdateOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/** C type : char* */
	public Pointer tableName;
	public GProMUpdateOperator() {
		super();
	}
	public GProMUpdateOperator(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "tableName");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param tableName C type : char*
	 */
	public GProMUpdateOperator(GProMQueryOperator op, Pointer tableName) {
		super();
		this.op = op;
		this.tableName = tableName;
	}
	public static class ByReference extends GProMUpdateOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMUpdateOperator implements Structure.ByValue {
		
	};
}
