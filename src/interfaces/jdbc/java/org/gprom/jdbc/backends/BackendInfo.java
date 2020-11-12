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
	public String getHost(String url) throws Exception;
	public String getPort(String url) throws Exception;
	public String getUser(String url) throws Exception;
	public String getPassword(String url) throws Exception;
	public String getDatabase(String url) throws Exception;
	
}
