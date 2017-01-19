package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
public class GProMNode extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	public GProMNode() {
		super();
	}
	public GProMNode(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public GProMNode(int type) {
		super();
		this.type = type;
	}
	public static class ByReference extends GProMNode implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMNode implements Structure.ByValue {
		
	};
}
