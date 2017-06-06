package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMOrderExpr extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference expr;
	/**
	 * @see GProMSortOrder<br>
	 * C type : GProMSortOrder
	 */
	public int order;
	/**
	 * @see GProMSortNullOrder<br>
	 * C type : GProMSortNullOrder
	 */
	public int nullOrder;
	public GProMOrderExpr() {
		super();
	}
	public GProMOrderExpr(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "expr", "order", "nullOrder");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param expr C type : GProMNode*<br>
	 * @param order @see GProMSortOrder<br>
	 * C type : GProMSortOrder<br>
	 * @param nullOrder @see GProMSortNullOrder<br>
	 * C type : GProMSortNullOrder
	 */
	public GProMOrderExpr(int type, org.gprom.jdbc.jna.GProMNode.ByReference expr, int order, int nullOrder) {
		super();
		this.type = type;
		this.expr = expr;
		this.order = order;
		this.nullOrder = nullOrder;
		write();
	}
	public static class ByReference extends GProMOrderExpr implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMOrderExpr implements Structure.ByValue {
		public ByValue(int type, org.gprom.jdbc.jna.GProMNode.ByReference expr, int order, int nullOrder){
			super(type,expr,order,nullOrder);
		}
		
	};
}
