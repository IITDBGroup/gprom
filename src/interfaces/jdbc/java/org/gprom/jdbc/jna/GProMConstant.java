package org.gprom.jdbc.jna;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMConstant extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/**
	 * @see GProMDataType<br>
	 * C type : GProMDataType
	 */
	public int constType;
	/** C type : void* */
	public Pointer value;
	public int isNull;
	public GProMConstant() {
		super();
	}
	public GProMConstant(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "constType", "value", "isNull");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param constType @see GProMDataType<br>
	 * C type : GProMDataType<br>
	 * @param value C type : void*
	 */
	public GProMConstant(int type, int constType, Pointer value, int isNull) {
		super();
		this.type = type;
		this.constType = constType;
		this.value = value;
		this.isNull = isNull;
		write();
	}
	public static class ByReference extends GProMConstant implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMConstant implements Structure.ByValue {
		public ByValue(int type, int constType, Pointer value, int isNull){
			super(type,constType,value,isNull);
		}
		
	};
}
