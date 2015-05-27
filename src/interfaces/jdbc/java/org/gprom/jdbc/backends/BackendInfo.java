/**
 * 
 */
package org.gprom.jdbc.backends;


/**
 * @author lord_pretzel
 *
 */
public interface BackendInfo {

	// helper functions to extract connection parameters from an url
	public String getHost(String url);
	public String getPort(String url);
	public String getUser(String url);
	public String getPassword(String url);
	public String getDatabase(String url);
}
