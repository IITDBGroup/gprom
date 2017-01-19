package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
public class GProMTableAccessOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/** C type : char* */
	public String tableName;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference asOf;
	public GProMTableAccessOperator() {
		super();
	}
	public GProMTableAccessOperator(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "tableName", "asOf");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param tableName C type : char*<br>
	 * @param asOf C type : GProMNode*
	 */
	public GProMTableAccessOperator(GProMQueryOperator op, String tableName, org.gprom.jdbc.jna.GProMNode.ByReference asOf) {
		super();
		this.op = op;
		this.tableName = tableName;
		this.asOf = asOf;
	}
	public static class ByReference extends GProMTableAccessOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMTableAccessOperator implements Structure.ByValue {
		
	};
}
