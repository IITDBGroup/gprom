package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMProvenanceComputation extends GProMStructure {
	/** C type : GProMQueryOperator */
	public GProMQueryOperator op;
	/**
	 * @see GProMProvenanceType<br>
	 * C type : GProMProvenanceType
	 */
	public int provType;
	/**
	 * @see GProMProvenanceInputType<br>
	 * C type : GProMProvenanceInputType
	 */
	public int inputType;
	/** C type : GProMProvenanceTransactionInfo* */
	public org.gprom.jdbc.jna.GProMProvenanceTransactionInfo.ByReference transactionInfo;
	/** C type : GProMNode* */
	public org.gprom.jdbc.jna.GProMNode.ByReference asOf;
	public GProMProvenanceComputation() {
		super();
	}
	public GProMProvenanceComputation(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("op", "provType", "inputType", "transactionInfo", "asOf");
	}
	/**
	 * @param op C type : GProMQueryOperator<br>
	 * @param provType @see GProMProvenanceType<br>
	 * C type : GProMProvenanceType<br>
	 * @param inputType @see GProMProvenanceInputType<br>
	 * C type : GProMProvenanceInputType<br>
	 * @param transactionInfo C type : GProMProvenanceTransactionInfo*<br>
	 * @param asOf C type : GProMNode*
	 */
	public GProMProvenanceComputation(GProMQueryOperator op, int provType, int inputType, org.gprom.jdbc.jna.GProMProvenanceTransactionInfo.ByReference transactionInfo, org.gprom.jdbc.jna.GProMNode.ByReference asOf) {
		super();
		this.op = op;
		this.provType = provType;
		this.inputType = inputType;
		this.transactionInfo = transactionInfo;
		this.asOf = asOf;
		write();
	}
	public static class ByReference extends GProMProvenanceComputation implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMProvenanceComputation implements Structure.ByValue {
		public ByValue(GProMQueryOperator op, int provType, int inputType, org.gprom.jdbc.jna.GProMProvenanceTransactionInfo.ByReference transactionInfo, org.gprom.jdbc.jna.GProMNode.ByReference asOf){
			super(op,provType,inputType,transactionInfo,asOf);
		}
		
	};
}
