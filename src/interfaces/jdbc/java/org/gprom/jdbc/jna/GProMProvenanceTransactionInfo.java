package org.gprom.jdbc.jna;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

public class GProMProvenanceTransactionInfo extends GProMStructure {
	/**
	 * @see GProMNodeTag<br>
	 * C type : GProMNodeTag
	 */
	public int type;
	/**
	 * @see GProMIsolationLevel<br>
	 * C type : GProMIsolationLevel
	 */
	public int transIsolation;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference updateTableNames;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference originalUpdates;
	/** C type : GProMList* */
	public org.gprom.jdbc.jna.GProMList.ByReference scns;
	/** C type : GProMConstant* */
	public org.gprom.jdbc.jna.GProMConstant.ByReference commitSCN;
	public GProMProvenanceTransactionInfo() {
		super();
	}
	public GProMProvenanceTransactionInfo(com.sun.jna.Pointer address){
		super(address);
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("type", "transIsolation", "updateTableNames", "originalUpdates", "scns", "commitSCN");
	}
	/**
	 * @param type @see GProMNodeTag<br>
	 * C type : GProMNodeTag<br>
	 * @param transIsolation @see GProMIsolationLevel<br>
	 * C type : GProMIsolationLevel<br>
	 * @param updateTableNames C type : GProMList*<br>
	 * @param originalUpdates C type : GProMList*<br>
	 * @param scns C type : GProMList*<br>
	 * @param commitSCN C type : GProMConstant*
	 */
	public GProMProvenanceTransactionInfo(int type, int transIsolation, org.gprom.jdbc.jna.GProMList.ByReference updateTableNames, org.gprom.jdbc.jna.GProMList.ByReference originalUpdates, org.gprom.jdbc.jna.GProMList.ByReference scns, org.gprom.jdbc.jna.GProMConstant.ByReference commitSCN) {
		super();
		this.type = type;
		this.transIsolation = transIsolation;
		this.updateTableNames = updateTableNames;
		this.originalUpdates = originalUpdates;
		this.scns = scns;
		this.commitSCN = commitSCN;
		write();
	}
	public static class ByReference extends GProMProvenanceTransactionInfo implements Structure.ByReference {
		
	};
	public static class ByValue extends GProMProvenanceTransactionInfo implements Structure.ByValue {
		public ByValue(int type, int transIsolation, org.gprom.jdbc.jna.GProMList.ByReference updateTableNames, org.gprom.jdbc.jna.GProMList.ByReference originalUpdates, org.gprom.jdbc.jna.GProMList.ByReference scns, org.gprom.jdbc.jna.GProMConstant.ByReference commitSCN){
			super(type,transIsolation,updateTableNames,originalUpdates,scns,commitSCN);
		}
		
	};
}
