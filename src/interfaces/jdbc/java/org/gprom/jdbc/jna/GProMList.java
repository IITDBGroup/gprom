package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMList extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	public int length;
	/** C type : GProMListCell* */
	public org.gprom.jdbc.jna.GProMListCell.ByReference head;
	/** C type : GProMListCell* */
	public org.gprom.jdbc.jna.GProMListCell.ByReference tail;
	public GProMList() {
		super();
	}
	public GProMList(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "length", "head", "tail");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param head C type : GProMListCell*<br>
	 * @param tail C type : GProMListCell*
	 */
	public GProMList(int type, int length, org.gprom.jdbc.jna.GProMListCell.ByReference head, org.gprom.jdbc.jna.GProMListCell.ByReference tail) {
		super();
		this.type = type;
		this.length = length;
		this.head = head;
		this.tail = tail;
		write();
	}
	public static class ByReference extends GProMList implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMList implements Structure.ByValue {
		public ByValue(int type, int length, org.gprom.jdbc.jna.GProMListCell.ByReference head, org.gprom.jdbc.jna.GProMListCell.ByReference tail){
			super(type,length,head,tail);
		}
		
	};
}
