package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Union;
import java.util.Arrays;
import java.util.List;

public class GProMListCell extends GProMStructure {
	/** C type : data_union */
	public data_union data;
	/** C type : GProMListCell* */
	public GProMListCell.ByReference next;
	/** <i>native declaration : line 10</i> */
	public static class data_union extends Union {
		/** C type : void* */
		public Pointer ptr_value;
		public int int_value;
		public data_union() {
			super();
		}
		/** @param ptr_value C type : void* */
		public data_union(Pointer ptr_value) {
			super();
			this.ptr_value = ptr_value;
			setType(Pointer.class);
			write();
		}

		public data_union(int int_value) {
			super();
			this.int_value = int_value;
			setType(Integer.TYPE);
			write();
		}

		public static class ByReference extends data_union implements Structure.ByReference {
			
		};
		public static class ByValue extends data_union implements Structure.ByValue {
			
			public ByValue(Pointer ptr_value) {
				super(ptr_value);
			}
			public ByValue(int int_value) {
				super(int_value);
			}
		};
	};
	public GProMListCell() {
		super();
	}
	public GProMListCell(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("data", "next");
	}
	/**
	 * @param data C type : data_union<br>
	 * @param next C type : GProMListCell*
	 */
	public GProMListCell(data_union data, GProMListCell.ByReference next) {
		super();
		this.data = data;
		this.next = next;
		write();
	}
	public static class ByReference extends GProMListCell implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMListCell implements Structure.ByValue {
		public ByValue(data_union data, GProMListCell.ByReference next){
			super(data,next);
		}
		
	};
}
