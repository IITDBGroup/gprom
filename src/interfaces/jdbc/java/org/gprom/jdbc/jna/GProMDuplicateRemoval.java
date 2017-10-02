package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMDuplicateRemoval extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/**
	 * attributes that need duplicate removal, GProMAttributeReference type<br>
	 * C type : GProMList*
	 */
	public org.gprom.jdbc.jna.GProMList.ByReference attrs;
	public GProMDuplicateRemoval() {
		super();
	}
	public GProMDuplicateRemoval(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "attrs");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param attrs attributes that need duplicate removal, GProMAttributeReference type<br>
	 * C type : GProMList*
	 */
	public GProMDuplicateRemoval(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference attrs) {
		super();
		this.op = op;
		this.attrs = attrs;
		write();
	}
	public static class ByReference extends GProMDuplicateRemoval implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMDuplicateRemoval implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference attrs){
			super(op,attrs);
		}
		
	};
}
