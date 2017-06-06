package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMCaseWhen extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference when;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference then;
	public GProMCaseWhen() {
		super();
	}
	public GProMCaseWhen(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "when", "then");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param when C type : GProMNode*<br>
	 * @param then C type : GProMNode*
	 */
	public GProMCaseWhen(int type, org.gprom.jdbc.jna.GProMNode.ByReference when, org.gprom.jdbc.jna.GProMNode.ByReference then) {
		super();
		this.type = type;
		this.when = when;
		this.then = then;
		write();
	}
	public static class ByReference extends GProMCaseWhen implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMCaseWhen implements Structure.ByValue {
		public ByValue(int type, org.gprom.jdbc.jna.GProMNode.ByReference when, org.gprom.jdbc.jna.GProMNode.ByReference then){
			super(type,when,then);
		}
		
	};
}
