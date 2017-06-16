/**
 * 
 */
package org.gprom.jdbc.pawd;

import org.json.JSONArray;
import org.json.JSONException;

/**
 * @author Amer
 *
 */
public interface VersionGraphStore {

	/**
	 * 
	 */

	public VersionGraph Load(JSONArray GraphJSONArray);
	public void Save(VersionGraph V)throws JSONException;
	//store as a JSON object
	public void Configure(VersionGraph V);
	//method for saving configuration
	public class Operation {
		String Code;
		OpType Op;
		public enum OpType{
			Query,
			Update
		}

	}

}
