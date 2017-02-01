package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMWindowFunction extends GProMStructure {
	public int type;
	/** C type : GProMFunctionCall* */
	public org.gprom.jdbc.jna.GProMFunctionCall.ByReference f;
	/** C type : GProMWindowDef* */
	public org.gprom.jdbc.jna.GProMWindowDef.ByReference win;
	public GProMWindowFunction() {
		super();
	}
	public GProMWindowFunction(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "f", "win");
	}
	/**
	 * @param f C type : GProMFunctionCall*<br>
	 * @param win C type : GProMWindowDef*
	 */
	public GProMWindowFunction(int type, org.gprom.jdbc.jna.GProMFunctionCall.ByReference f, org.gprom.jdbc.jna.GProMWindowDef.ByReference win) {
		super();
		this.type = type;
		this.f = f;
		this.win = win;
	}
	public static class ByReference extends GProMWindowFunction implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMWindowFunction implements Structure.ByValue {
		
	};
}
