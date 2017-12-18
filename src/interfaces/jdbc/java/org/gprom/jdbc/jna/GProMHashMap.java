package org.gprom.jdbc.jna;
import com.sun.jna.Callback;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMHashMap extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int keyType;
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int valueType;
	public int typelen;
	/** C type : eq_callback* */
	public GProMHashMap.eq_callback eq;
	/** C type : cpy_callback* */
	public GProMHashMap.cpy_callback cpy;
	/** C type : GProMHashElem* */
	public org.gprom.jdbc.jna.GProMHashElem.ByReference elem;
	public interface eq_callback extends Callback {
		int apply(Pointer voidPtr1, Pointer voidPtr2);
	};
	public interface cpy_callback extends Callback {
		Pointer apply(Pointer voidPtr1);
	};
	public GProMHashMap() {
		super();
	}
	public GProMHashMap(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "keyType", "valueType", "typelen", "eq", "cpy", "elem");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param keyType @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param valueType @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param eq C type : eq_callback*<br>
	 * @param cpy C type : cpy_callback*<br>
	 * @param elem C type : GProMHashElem*
	 */
	public GProMHashMap(int type, int keyType, int valueType, int typelen, GProMHashMap.eq_callback eq, GProMHashMap.cpy_callback cpy, org.gprom.jdbc.jna.GProMHashElem.ByReference elem) {
		super();
		this.type = type;
		this.keyType = keyType;
		this.valueType = valueType;
		this.typelen = typelen;
		this.eq = eq;
		this.cpy = cpy;
		this.elem = elem;
		write();
	}
	public static class ByReference extends GProMHashMap implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMHashMap implements Structure.ByValue {
		public ByValue(int type, int keyType, int valueType, int typelen, GProMHashMap.eq_callback eq, GProMHashMap.cpy_callback cpy, org.gprom.jdbc.jna.GProMHashElem.ByReference elem){
			super(type,keyType,valueType,typelen,eq,cpy,elem);
		}
		
	};
}
