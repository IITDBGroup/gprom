package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMRowNumExpr extends GProMStructure {
	public int type;
	public GProMRowNumExpr() {
		super();
	}
	public GProMRowNumExpr(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type");
	}
	public GProMRowNumExpr(int type) {
		super();
		this.type = type;
		write();
	}
	public static class ByReference extends GProMRowNumExpr implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMRowNumExpr implements Structure.ByValue {
		public ByValue(int type) {
			super(type);
		}
	};
}
