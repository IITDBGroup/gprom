/**
 * 
 */
package org.gprom.jdbc.pawd;



import org.json.JSONObject;
/**
 * @author Amer
 *
 */
public interface VersionGraphStore {//TODO split this into VersionGraphManager and VersionGraphStore 
	//and make store more generic load without parameters, 
	//save with only VersionGraph and config(Properties options) 
	//to set store specific parameters, e.g., JSON file path

	/**
	 * 
	 */

	public VersionGraph Load(JSONObject GraphJSONArray);
	public JSONObject Save(VersionGraph V);
	//method for saving configuration
	public class Operation {
		String Code;
		OpType Op;
		public Operation(String code, OpType op) {
			Code = code;
			Op = op;
		}
		/**
		 * @return the code
		 */
		public String getCode() {
			return Code;
		}
		/**
		 * @param code the code to set
		 */
		public void setCode(String code) {
			Code = code;
		}
		/**
		 * @return the op
		 */
		public OpType getOp() {
			return Op;
		}
		/**
		 * @param op the op to set
		 */
		public void setOp(OpType op) {
			Op = op;
		}
		/* (non-Javadoc)
		 * @see java.lang.Object#toString()
		 */
		@Override
		public String toString() {
			return "[Code=" + Code + ", Op=" + Op + "]";
		}

		public enum OpType{
			Query,
			Update
		}
		public enum Materialization{
			isMaterialized,
			notMaterialized
		}
		@Override 
		public boolean equals(Object other){
		    if (other == null) return false;
		    if (other == this) return true;
		    if (!(other instanceof Operation))return false;
		    Operation otherOp = (Operation)other;
		    if(otherOp.getCode().equals(Code) && otherOp.Op.equals(Op)) return true;
		    return false;
		}

	}

}
