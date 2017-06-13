/**
 * 
 */
package org.gprom.jdbc.pawd;
/**
 * @author Amer
 *
 */
public interface VersionGraphStore {

	/**
	 * 
	 */
	public VersionGraph Load();
	public void Save(VersionGraph V);
	public void Configure(VersionGraph V);
}
