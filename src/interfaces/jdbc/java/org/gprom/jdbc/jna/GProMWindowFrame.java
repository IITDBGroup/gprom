package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMWindowFrame extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
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
	public GProMWindowFrame(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "frameType", "lower", "higher");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
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
		write();
	}
	public static class ByReference extends GProMWindowFrame implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMWindowFrame implements Structure.ByValue {
		public ByValue(int type, int frameType, org.gprom.jdbc.jna.GProMWindowBound.ByReference lower, org.gprom.jdbc.jna.GProMWindowBound.ByReference higher){
			super(type,frameType,lower,higher);
		}
		
	};
}
