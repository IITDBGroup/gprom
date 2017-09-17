
package org.gprom.jdbc.pawd;



import java.util.Properties;
/**
 * @author Amer
 *
 */
interface VersionGraphStore {


	/**
	 * 
	 */

    void initialize(Properties options) throws IllegalArgumentException;
	VersionGraph load(String versionGraphId);
	void save(VersionGraph g);

}
