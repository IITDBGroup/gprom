package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMWindowFunction extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : GProMFunctionCall* */
	public org.gprom.jdbc.jna.GProMFunctionCall.ByReference f;
	/** C type : GProMWindowDef* */
	public org.gprom.jdbc.jna.GProMWindowDef.ByReference win;
	public GProMWindowFunction() {
		super();
	}
	public GProMWindowFunction(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "f", "win");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param f C type : GProMFunctionCall*<br>
	 * @param win C type : GProMWindowDef*
	 */
	public GProMWindowFunction(int type, org.gprom.jdbc.jna.GProMFunctionCall.ByReference f, org.gprom.jdbc.jna.GProMWindowDef.ByReference win) {
		super();
		this.type = type;
		this.f = f;
		this.win = win;
		write();
	}
	public static class ByReference extends GProMWindowFunction implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMWindowFunction implements Structure.ByValue {
		public ByValue(int type, org.gprom.jdbc.jna.GProMFunctionCall.ByReference f, org.gprom.jdbc.jna.GProMWindowDef.ByReference win){
			super(type,f,win);
		}
		
	};
}
