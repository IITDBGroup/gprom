/**
 * 
 */
package org.gprom.jdbc.pawd;


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
	public JSONObject Save(VersionGraph V);
	//store as a JSON object
	public void Configure(VersionGraph V);
	//method for saving configuration
	public class Operation {
		String Code;
		/* (non-Javadoc)
		 * @see java.lang.Object#toString()
		 */
		@Override
		public String toString() {
			return "Operation [Code=" + Code + ", Op=" + Op + "]";
		}
		OpType Op;
		public enum OpType{
			Query,
			Update
		}

	}

}
