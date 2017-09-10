package org.gprom.jdbc.pawd;

import org.stringtemplate.v4.*;

import java.io.File;
import java.util.Properties;




/**
 * @author Amer
 *
 */
public class JSONVersionGraphStore implements VersionGraphStore {

	public static final String PROP_STORE_DIR = "dir";
	
	private File storeDir;

	

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.pawd.VersionGraphStore#initialize(java.util.Properties)
	 */
	@Override
	public void initialize(Properties options) throws IllegalArgumentException {
		if (options.getProperty(PROP_STORE_DIR) == null)
			throw new IllegalArgumentException ("need to provide option " + PROP_STORE_DIR);
		storeDir = new File(options.getProperty(PROP_STORE_DIR));
		if (!storeDir.isDirectory())
			throw new IllegalArgumentException ("dir " + storeDir + " does not exist");
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.pawd.VersionGraphStore#load(java.lang.String)
	 */
	@Override
	public VersionGraph load(String versionGraphId) throws Exception {
		File graphFile = new File(storeDir, createFileName(versionGraphId));
		VersionGraph g = null;
		//TODO 
		return g;
	}
	/**
	 * @param versionGraphId
	 * @return
	 */
	private String createFileName(String versionGraphId) {
		return versionGraphId + ".json";
	}
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.pawd.VersionGraphStore#save(org.gprom.jdbc.pawd.VersionGraph)
	 */
	@Override
	public void save(VersionGraph g) throws Exception {
		// TODO Auto-generated method stub
		
	}




}
