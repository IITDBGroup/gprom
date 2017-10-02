package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMQueryOperator extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/**
	 * children of the operator node, GProMQueryOperator type<br>
	 * C type : GProMList*
	 */
	public org.gprom.jdbc.jna.GProMList.ByReference inputs;
	/**
	 * attributes and their data types of result tables, GProMSchema type<br>
	 * C type : GProMSchema*
	 */
	public org.gprom.jdbc.jna.GProMSchema.ByReference schema;
	/**
	 * direct parents of the operator node, GProMQueryOperator type<br>
	 * C type : GProMList*
	 */
	public org.gprom.jdbc.jna.GProMList.ByReference parents;
	/**
	 * positions of provenance attributes in the operator's schema<br>
	 * C type : GProMList*
	 */
	public org.gprom.jdbc.jna.GProMList.ByReference provAttrs;
	/**
	 * generic node to store flexible list or map of properties (GProMKeyValue) for query operators<br>
	 * C type : GProMNode*
	 */
	public org.gprom.jdbc.jna.GProMNode.ByReference properties;
	public GProMQueryOperator() {
		super();
	}
	public GProMQueryOperator(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "inputs", "schema", "parents", "provAttrs", "properties");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param inputs children of the operator node, GProMQueryOperator type<br>
	 * C type : GProMList*<br>
	 * @param schema attributes and their data types of result tables, GProMSchema type<br>
	 * C type : GProMSchema*<br>
	 * @param parents direct parents of the operator node, GProMQueryOperator type<br>
	 * C type : GProMList*<br>
	 * @param provAttrs positions of provenance attributes in the operator's schema<br>
	 * C type : GProMList*<br>
	 * @param properties generic node to store flexible list or map of properties (GProMKeyValue) for query operators<br>
	 * C type : GProMNode*
	 */
	public GProMQueryOperator(int type, org.gprom.jdbc.jna.GProMList.ByReference inputs, org.gprom.jdbc.jna.GProMSchema.ByReference schema, org.gprom.jdbc.jna.GProMList.ByReference parents, org.gprom.jdbc.jna.GProMList.ByReference provAttrs, org.gprom.jdbc.jna.GProMNode.ByReference properties) {
		super();
		this.type = type;
		this.inputs = inputs;
		this.schema = schema;
		this.parents = parents;
		this.provAttrs = provAttrs;
		this.properties = properties;
		write();
	}
	public static class ByReference extends GProMQueryOperator implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMQueryOperator implements Structure.ByValue {
		public ByValue(int type, org.gprom.jdbc.jna.GProMList.ByReference inputs, org.gprom.jdbc.jna.GProMSchema.ByReference schema, org.gprom.jdbc.jna.GProMList.ByReference parents, org.gprom.jdbc.jna.GProMList.ByReference provAttrs, org.gprom.jdbc.jna.GProMNode.ByReference properties){
			super(type,inputs,schema,parents,provAttrs,properties);
		}
		
	};
}
