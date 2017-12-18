package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMHashElem extends GProMStructure {
	/** C type : void* */
	public Pointer data;
	/** C type : void* */
	public Pointer key;
	/** C type : GProMUT_hash_handle */
	public GProMUT_hash_handle hh;
	public GProMHashElem() {
		super();
	}
	public GProMHashElem(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("data", "key", "hh");
	}
	/**
	 * @param data C type : void*<br>
	 * @param key C type : void*<br>
	 * @param hh C type : GProMUT_hash_handle
	 */
	public GProMHashElem(Pointer data, Pointer key, GProMUT_hash_handle hh) {
		super();
		this.data = data;
		this.key = key;
		this.hh = hh;
		write();
	}
	public static class ByReference extends GProMHashElem implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMHashElem implements Structure.ByValue {
		public ByValue(Pointer data, Pointer key, GProMUT_hash_handle hh){
			super(data,key,hh);
		}
		
	};
}
