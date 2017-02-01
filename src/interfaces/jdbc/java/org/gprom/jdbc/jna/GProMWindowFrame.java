package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMWindowFrame extends GProMStructure {
	public int type;
	/**
	 * @see GProMWinFrameType<br>
	 * C type : GProMWinFrameType
	 */
	public int frameType;
	/** C type : GProMWindowBound* */
	public org.gprom.jdbc.jna.GProMWindowBound.ByReference lower;
	/** C type : GProMWindowBound* */
	public org.gprom.jdbc.jna.GProMWindowBound.ByReference higher;
	public GProMWindowFrame() {
		super();
	}
	public GProMWindowFrame(Pointer address) {
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "frameType", "lower", "higher");
	}
	/**
	 * @param frameType @see GProMWinFrameType<br>
	 * C type : GProMWinFrameType<br>
	 * @param lower C type : GProMWindowBound*<br>
	 * @param higher C type : GProMWindowBound*
	 */
	public GProMWindowFrame(int type, int frameType, org.gprom.jdbc.jna.GProMWindowBound.ByReference lower, org.gprom.jdbc.jna.GProMWindowBound.ByReference higher) {
		super();
		this.type = type;
		this.frameType = frameType;
		this.lower = lower;
		this.higher = higher;
	}
	public static class ByReference extends GProMWindowFrame implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMWindowFrame implements Structure.ByValue {
		
	};
}
