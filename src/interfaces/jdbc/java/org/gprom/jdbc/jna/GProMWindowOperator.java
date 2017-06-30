package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;


public class GProMWindowOperator extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference partitionBy;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference orderBy;
	/** C type : GProMWindowFrame* */
	public org.gprom.jdbc.jna.GProMWindowFrame.ByReference frameDef;
	/** C type : String */
	public String attrName;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference f;
	public GProMWindowOperator() {
		super();
	}
	public GProMWindowOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "partitionBy", "orderBy", "frameDef", "attrName", "f");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param partitionBy C type : GProMList*<br>
	 * @param orderBy C type : GProMList*<br>
	 * @param frameDef C type : GProMWindowFrame*<br>
	 * @param attrName C type : String<br>
	 * @param f C type : GProMNode*
	 */
	public GProMWindowOperator(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference partitionBy, org.gprom.jdbc.jna.GProMList.ByReference orderBy, org.gprom.jdbc.jna.GProMWindowFrame.ByReference frameDef, String attrName, org.gprom.jdbc.jna.GProMNode.ByReference f) {
		super();
		this.op = op;
		this.partitionBy = partitionBy;
		this.orderBy = orderBy;
		this.frameDef = frameDef;
		this.attrName = attrName;
		this.f = f;
		write();
	}
	public static class ByReference extends GProMWindowOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMWindowOperator implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, org.gprom.jdbc.jna.GProMList.ByReference partitionBy, org.gprom.jdbc.jna.GProMList.ByReference orderBy, org.gprom.jdbc.jna.GProMWindowFrame.ByReference frameDef, String attrName, org.gprom.jdbc.jna.GProMNode.ByReference f){
			super(op,partitionBy,orderBy,frameDef,attrName,f);
		}
		
	};
}
