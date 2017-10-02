package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;


public class GProMSQLParameter extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/** C type : String */
	public String name;
	public int position;
	/**
	 * @see GProMDataType<br>
	 * C type : GProMDataType
	 */
	public int parType;
	public GProMSQLParameter() {
		super();
	}
	public GProMSQLParameter(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "name", "position", "parType");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param name C type : String<br>
	 * @param parType @see GProMDataType<br>
	 * C type : GProMDataType
	 */
	public GProMSQLParameter(int type, String name, int position, int parType) {
		super();
		this.type = type;
		this.name = name;
		this.position = position;
		this.parType = parType;
		write();
	}
	public static class ByReference extends GProMSQLParameter implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMSQLParameter implements Structure.ByValue {
		public ByValue(int type, String name, int position, int parType){
			super(type,name,position,parType);
		}
		
	};
}
