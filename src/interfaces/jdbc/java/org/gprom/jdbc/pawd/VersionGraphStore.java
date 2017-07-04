/**
 * 
 */
package org.gprom.jdbc.pawd;



import java.util.ArrayList;

import org.json.JSONObject;
/**
 * @author Amer
 *
 */
public interface VersionGraphStore {

	/**
	 * 
	 */

	public VersionGraph Load(JSONObject GraphJSONArray);
	public void UpdateCall(VersionGraph V, Node s);
	public JSONObject Save(VersionGraph V);
	public String Compose(VersionGraph V, Node s);
	public ArrayList<VersionGraph> getPath(VersionGraph V,Node n);
	//public VersionGraph genPathes(ArrayList<VersionGraph>Graphs,VersionGraph V,Node t);
	//store as a JSON object
	public void Configure(VersionGraph V);
	//method for saving configuration
	public class Operation {
		String Code;
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
		OpType Op;
		public enum OpType{
			Query,
			Update
		}

	}

}
